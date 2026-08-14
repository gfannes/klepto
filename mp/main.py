from mpos import Activity
import lvgl as lv

import network
import espnow
import ubinascii
import struct
import time

ROVER_IDS = "ABCDEFGHIJK"
BROADCAST_MAC = b"\xff\xff\xff\xff\xff\xff"
ESP_NOW_CHANNEL = 1
CENTER = 2048
MIN_AXIS = 0
MAX_AXIS = 4095
SEND_INTERVAL_MS = 100
AXIS_HOLD_MS = 700
BUTTON_HOLD_MS = 350
SELECT_HOLD_MS = 1500
BUTTON_KEYS = {
    10: "a",  # A button: shoot
    13: "a",
    3: "y",  # S button: double shoot control; current rover firmware shoots 3
}

wlan = network.WLAN(network.STA_IF)


def setup_wifi_for_espnow():
    try:
        network.WLAN(network.AP_IF).active(False)
    except Exception:
        pass

    wlan.active(False)
    wlan.active(True)
    try:
        wlan.disconnect()
    except Exception:
        pass
    try:
        wlan.config(pm=wlan.PM_NONE)
    except Exception:
        pass
    try:
        wlan.config(channel=ESP_NOW_CHANNEL)
    except Exception:
        pass


setup_wifi_for_espnow()

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

        mac_label = lv.label(screen)
        mac_label.align(lv.ALIGN.CENTER, 0, 60)
        mac_label.set_text("badge %s\nbroadcast ch %d" % (badge_mac, ESP_NOW_CHANNEL))

        status = lv.label(screen)
        status.align(lv.ALIGN.CENTER, 0, -10)
        status.set_text("Select rover")

        key_label = lv.label(screen)
        key_label.align(lv.ALIGN.CENTER, 0, 28)
        key_label.set_text("key: -")

        send_label = lv.label(screen)
        send_label.align(lv.ALIGN.CENTER, 0, 44)
        send_label.set_text("sent: 0 tx: 0 fail: 0")

        try:
            e = espnow.ESPNow()
            e.active(True)

            try:
                e.add_peer(BROADCAST_MAC, channel=ESP_NOW_CHANNEL)
            except TypeError:
                e.add_peer(BROADCAST_MAC)
        except Exception as exc:
            label.set_text("Robot offline")
            status.set_text("Cannot add ESP-NOW peer\n%s" % exc)
            self.setContentView(screen)
            return

        selected_rover_index = 0
        rover_selected = False
        select_expires = ticks_ms() + SELECT_HOLD_MS
        axes = {"forward": CENTER, "steer": CENTER}
        axis_expires = {"forward": 0, "steer": 0}
        button_expires = {"a": 0, "b": 0, "x": 0, "y": 0}
        buttons = {"a": 0, "b": 0, "x": 0, "y": 0}
        last_packet = None
        send_count = 0
        tx_count = 0
        fail_count = 0

        def selected_rover_id():
            return ROVER_IDS[selected_rover_index]

        def update_select_screen():
            label.set_text("Rover %s" % selected_rover_id())
            status.set_text("Select rover\nleft/right, wait")

        def start_control():
            nonlocal rover_selected, last_packet
            rover_selected = True
            last_packet = None
            label.set_text("Robot control")
            send_state(True)

        def make_packet():
            return struct.pack(
                "<hhBBBBB",
                axes["forward"],
                axes["steer"],
                buttons["a"],
                buttons["b"],
                buttons["x"],
                buttons["y"],
                ord(selected_rover_id()),
            )

        def describe_state():
            return "Rover %s F=%d S=%d\nshoot=%d dbl=%d up=%d down=%d" % (
                selected_rover_id(),
                axes["forward"],
                axes["steer"],
                buttons["a"],
                buttons["y"],
                buttons["x"],
                buttons["b"],
            )

        def send_state(force=False):
            nonlocal last_packet, send_count, tx_count, fail_count
            if not rover_selected:
                return
            packet = make_packet()
            changed = packet != last_packet
            if not force and packet == last_packet:
                return
            try:
                e.send(BROADCAST_MAC, packet)
            except Exception as exc:
                fail_count += 1
                send_label.set_text("sent: %d tx: %d fail: %d" % (send_count, tx_count, fail_count))
                status.set_text("Send failed\n%s" % exc)
                return
            last_packet = packet
            tx_count += 1
            if changed:
                send_count += 1
            send_label.set_text("sent: %d tx: %d fail: %d" % (send_count, tx_count, fail_count))
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
            nonlocal rover_selected
            if not rover_selected and ticks_diff(ticks_ms(), select_expires) >= 0:
                start_control()
            update_axes()
            update_buttons()
            send_state(True)

        def on_key(event):
            nonlocal selected_rover_index, select_expires
            key = event.get_key()
            now = ticks_ms()
            key_label.set_text("key: %s name: %s" % (key, key_name(key)))
            if not rover_selected:
                if key in (lv.KEY.LEFT, lv.KEY.DOWN):
                    selected_rover_index = (selected_rover_index - 1) % len(ROVER_IDS)
                    select_expires = now + SELECT_HOLD_MS
                    update_select_screen()
                elif key in (lv.KEY.RIGHT, lv.KEY.UP):
                    selected_rover_index = (selected_rover_index + 1) % len(ROVER_IDS)
                    select_expires = now + SELECT_HOLD_MS
                    update_select_screen()
                return

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
        update_select_screen()
        
