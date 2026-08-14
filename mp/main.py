from mpos import Activity
import lvgl as lv

import network
import espnow
import ubinascii
import binascii
import struct

wlan = network.WLAN(network.STA_IF)
wlan.active(True)

PEER_MAC = "B0CBD88975E8"
CENTER = 2048
MIN_AXIS = 0
MAX_AXIS = 4095
BUTTON_KEYS = {
    "a": "a",  # shoot
    "b": "b",  # tilt down
    "s": "x",  # double shoot
    "y": "y",  # tilt up
}


def axis_value(negative, positive):
    if negative and not positive:
        return MIN_AXIS
    if positive and not negative:
        return MAX_AXIS
    return CENTER


def key_name(key):
    if isinstance(key, int) and 0 <= key <= 255:
        return chr(key).lower()
    return key


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
        status.align(lv.ALIGN.CENTER, 0, 0)
        status.set_text("Connecting...")

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

        keys_down = {
            lv.KEY.LEFT: False,
            lv.KEY.RIGHT: False,
            lv.KEY.UP: False,
            lv.KEY.DOWN: False,
        }
        buttons = {"a": 0, "b": 0, "x": 0, "y": 0}
        last_packet = None

        def make_packet():
            left_right = axis_value(keys_down[lv.KEY.LEFT], keys_down[lv.KEY.RIGHT])
            fwd_bckwd = axis_value(keys_down[lv.KEY.DOWN], keys_down[lv.KEY.UP])
            return struct.pack(
                "<hhBBBB",
                left_right,
                fwd_bckwd,
                buttons["a"],
                buttons["b"],
                buttons["x"],
                buttons["y"],
            )

        def describe_state():
            lr = axis_value(keys_down[lv.KEY.LEFT], keys_down[lv.KEY.RIGHT])
            fb = axis_value(keys_down[lv.KEY.DOWN], keys_down[lv.KEY.UP])
            return "LR=%d FB=%d\nshoot=%d dbl=%d up=%d down=%d" % (
                lr,
                fb,
                buttons["a"],
                buttons["x"],
                buttons["y"],
                buttons["b"],
            )

        def send_state():
            nonlocal last_packet
            packet = make_packet()
            if packet == last_packet:
                return
            try:
                e.send(peer, packet)
            except Exception as exc:
                status.set_text("Send failed\n%s" % exc)
                return
            last_packet = packet
            status.set_text(describe_state())

        def on_key(event):
            key = event.get_key()
            if key in keys_down:
                keys_down[key] = not keys_down[key]
            elif key_name(key) in BUTTON_KEYS:
                name = BUTTON_KEYS[key_name(key)]
                buttons[name] = 0 if buttons[name] else 1
            else:
                return

            label.set_text("Robot control")
            send_state()

        lv.group_get_default().add_obj(screen)
        lv.group_focus_obj(screen)
        screen.add_event_cb(on_key, lv.EVENT.KEY, None)

        self.setContentView(screen)
        send_state()
        
