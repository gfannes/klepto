from mpos import Activity
import lvgl as lv

import network
import espnow
import ubinascii
import binascii
import struct
import time

wlan = network.WLAN(network.STA_IF)
wlan.active(True)

PEER_MAC = "B0CBD88975E8"
CENTER = 2048
MIN_AXIS = 0
MAX_AXIS = 4095
SEND_INTERVAL_MS = 100
AXIS_HOLD_MS = 700
BUTTON_HOLD_MS = 350
BUTTON_KEYS = {
    10: "a",  # A button: shoot
    3: "y",  # S button: double shoot control; current rover firmware shoots 3
}

def ticks_ms():
    if hasattr(time, "ticks_ms"):
        return time.ticks_ms()
    return int(time.time() * 1000)


def ticks_diff(left, right):
    if hasattr(time, "ticks_diff"):
        return time.ticks_diff(left, right)
    return left - right


def key_name(key):
    if isinstance(key, int) and 32 <= key <= 126:
        return chr(key).lower()
    if isinstance(key, str):
        return key.lower()
    return str(key)


def button_field_for_key(key):
    if key in BUTTON_KEYS:
        return BUTTON_KEYS[key]
    return BUTTON_KEYS.get(key_name(key))


class Main(Activity):
    def onCreate(self):
        print("App started")
        screen = lv.obj()
        label = lv.label(screen)
        label.set_text("Klepto")
        label.align(lv.ALIGN.CENTER, 0, -60)

        badge_mac = ubinascii.hexlify(wlan.config("mac"), ":").decode()
        rover_mac = ":".join(PEER_MAC[i:i + 2] for i in range(0, len(PEER_MAC), 2)).lower()

        mac_label = lv.label(screen)
        mac_label.align(lv.ALIGN.CENTER, 0, 60)
        mac_label.set_text("badge %s\nrover %s" % (badge_mac, rover_mac))

        status = lv.label(screen)
        status.align(lv.ALIGN.CENTER, 0, -10)
        status.set_text("Connecting...")

        key_label = lv.label(screen)
        key_label.align(lv.ALIGN.CENTER, 0, 28)
        key_label.set_text("key: -")

        try:
            e = espnow.ESPNow()
            e.active(True)

            peer = binascii.unhexlify(PEER_MAC)
            e.add_peer(peer)
        except Exception as exc:
            label.set_text("Robot offline")
            status.set_text("Cannot add ESP-NOW peer\n%s" % exc)
            self.setContentView(screen)
            return

        axes = {"forward": CENTER, "steer": CENTER}
        axis_expires = {"forward": 0, "steer": 0}
        button_expires = {"a": 0, "b": 0, "x": 0, "y": 0}
        buttons = {"a": 0, "b": 0, "x": 0, "y": 0}
        last_packet = None

        def make_packet():
            return struct.pack(
                "<hhBBBB",
                axes["forward"],
                axes["steer"],
                buttons["a"],
                buttons["b"],
                buttons["x"],
                buttons["y"],
            )

        def describe_state():
            return "F=%d S=%d\nshoot=%d dbl=%d up=%d down=%d" % (
                axes["forward"],
                axes["steer"],
                buttons["a"],
                buttons["y"],
                buttons["x"],
                buttons["b"],
            )

        def send_state(force=False):
            nonlocal last_packet
            packet = make_packet()
            if not force and packet == last_packet:
                return
            try:
                e.send(peer, packet)
            except Exception as exc:
                status.set_text("Send failed\n%s" % exc)
                return
            last_packet = packet
            status.set_text(describe_state())

        def update_buttons():
            now = ticks_ms()
            for name in buttons:
                buttons[name] = 1 if ticks_diff(button_expires[name], now) > 0 else 0

        def update_axes():
            now = ticks_ms()
            for name in axes:
                if ticks_diff(axis_expires[name], now) <= 0:
                    axes[name] = CENTER

        def heartbeat(timer):
            update_axes()
            update_buttons()
            send_state(True)

        def on_key(event):
            key = event.get_key()
            now = ticks_ms()
            key_label.set_text("key: %s name: %s" % (key, key_name(key)))
            if key == lv.KEY.UP:
                axes["forward"] = MAX_AXIS
                axis_expires["forward"] = now + AXIS_HOLD_MS
            elif key == lv.KEY.DOWN:
                axes["forward"] = MIN_AXIS
                axis_expires["forward"] = now + AXIS_HOLD_MS
            elif key == lv.KEY.LEFT:
                axes["steer"] = MIN_AXIS
                axis_expires["steer"] = now + AXIS_HOLD_MS
            elif key == lv.KEY.RIGHT:
                axes["steer"] = MAX_AXIS
                axis_expires["steer"] = now + AXIS_HOLD_MS
            else:
                name = button_field_for_key(key)
                if name is None:
                    return
                button_expires[name] = now + BUTTON_HOLD_MS
                buttons[name] = 1

            label.set_text("Robot control")
            send_state(True)

        lv.group_get_default().add_obj(screen)
        lv.group_focus_obj(screen)
        screen.add_event_cb(on_key, lv.EVENT.KEY, None)
        lv.timer_create(heartbeat, SEND_INTERVAL_MS, None)

        self.setContentView(screen)
        send_state(True)
        
