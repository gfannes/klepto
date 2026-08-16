from mpos import Activity
import mpos
import lvgl as lv

# Desktop MicroPythonOS (lvgl_micropy_unix) has no ESP32 Wi-Fi stack.
# Importing network/espnow there raised ImportError before onCreate ran, so
# the UI could not be launched on a PC. On the badge these modules exist and
# behavior is unchanged. wlan, setup_wifi_for_espnow, MAC display, and
# reset_espnow all no-op / show "unavailable" when the modules are missing.
try:
    import network
    import espnow
except ImportError:
    network = None
    espnow = None
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
GRID_COLUMNS = 4
LETTER_BTN_W = 56
LETTER_BTN_H = 40
BUTTON_KEYS = {
    10: "a",  # A button: shoot
    13: "a",
    3: "s",  # S button: double shoot control
}
EXPANDER_BUTTONS = {
    "b": 6,
    "x": 8,  # Physical Y button: tilt up command.
}

wlan = network.WLAN(network.STA_IF) if network is not None else None


def setup_wifi_for_espnow():
    if network is None or wlan is None:
        return

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


def read_expander_digital():
    try:
        return tuple(mpos.io_expander.digital)
    except Exception:
        return None


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
        try:
            screen.remove_flag(lv.obj.FLAG.SCROLLABLE)
        except Exception:
            pass

        title = lv.label(screen)
        title.set_text("Klepto Mars Rover Controller")
        title.set_width(lv.pct(94))
        try:
            title.set_long_mode(lv.label.LONG.WRAP)
        except Exception:
            pass
        title.set_style_text_align(lv.TEXT_ALIGN.CENTER, 0)
        try:
            title.set_style_text_font(lv.font_montserrat_16, lv.PART.MAIN)
            title.set_style_text_letter_space(1, lv.PART.MAIN)
        except Exception:
            pass
        title.align(lv.ALIGN.TOP_MID, 0, 10)

        instruction = lv.label(screen)
        instruction.set_text("Choose your rover")
        instruction.set_width(lv.pct(94))
        instruction.set_style_text_align(lv.TEXT_ALIGN.CENTER, 0)
        instruction.align_to(title, lv.ALIGN.OUT_BOTTOM_MID, 0, 8)

        grid = lv.obj(screen)
        grid.set_width(lv.pct(94))
        grid.set_style_bg_opa(lv.OPA.TRANSP, 0)
        grid.set_style_border_width(0, 0)
        grid.set_style_pad_all(4, 0)
        grid.set_style_pad_row(6, 0)
        grid.set_style_pad_column(6, 0)
        try:
            grid.set_flex_flow(lv.FLEX_FLOW.ROW_WRAP)
            grid.set_flex_align(lv.FLEX_ALIGN.CENTER, lv.FLEX_ALIGN.CENTER, lv.FLEX_ALIGN.CENTER)
            grid.set_height(lv.SIZE_CONTENT)
        except Exception:
            grid.set_height(LETTER_BTN_H * 3 + 24)
        grid.align_to(instruction, lv.ALIGN.OUT_BOTTOM_MID, 0, 8)

        def badge_mac_text():
            # No STA interface on desktop; keep the debug line filled in.
            if wlan is None:
                return "desktop"
            try:
                return ubinascii.hexlify(wlan.config("mac"), ":").decode()
            except Exception:
                return "unknown"

        badge_mac = badge_mac_text()

        status = lv.label(screen)
        status.set_width(lv.pct(94))
        status.set_style_text_align(lv.TEXT_ALIGN.CENTER, 0)
        status.set_text("")

        key_label = lv.label(screen)
        key_label.set_width(lv.pct(94))
        key_label.set_style_text_align(lv.TEXT_ALIGN.CENTER, 0)
        key_label.set_text("key: -")

        send_label = lv.label(screen)
        send_label.set_width(lv.pct(94))
        send_label.set_style_text_align(lv.TEXT_ALIGN.CENTER, 0)
        send_label.set_text("sent: 0 tx: 0 fail: 0")

        mac_label = lv.label(screen)
        mac_label.set_width(lv.pct(94))
        mac_label.set_style_text_align(lv.TEXT_ALIGN.CENTER, 0)
        mac_label.set_text("badge %s\nbroadcast ch %d" % (badge_mac, ESP_NOW_CHANNEL))

        help_box = lv.obj(screen)
        help_box.set_width(lv.pct(94))
        help_box.set_style_border_width(1, 0)
        help_box.set_style_pad_all(8, 0)
        help_box.set_style_radius(6, 0)
        help_box.set_style_bg_opa(lv.OPA.TRANSP, 0)
        try:
            help_box.set_height(lv.SIZE_CONTENT)
        except Exception:
            help_box.set_height(64)

        help_label = lv.label(help_box)
        help_label.set_width(lv.pct(100))
        help_label.set_style_text_align(lv.TEXT_ALIGN.CENTER, 0)
        help_label.set_text(
            "Joystick: move the rover\n"
            "Y: aim up   B: aim down\n"
            "A: shoot once   S: burst (3)"
        )

        debug_widgets = (status, key_label, send_label, mac_label, help_box)
        for widget in debug_widgets:
            widget.add_flag(lv.obj.FLAG.HIDDEN)

        e = None

        def reset_espnow(report_errors=True):
            nonlocal e
            if espnow is None:
                # PC simulator: skip radio init; send_state already treats a
                # failed reset as a fail count plus debug text.
                e = None
                if report_errors:
                    send_label.set_text("ESP-NOW unavailable")
                return False
            if e is not None:
                try:
                    e.active(False)
                except Exception:
                    pass
            try:
                setup_wifi_for_espnow()
                e = espnow.ESPNow()
                e.active(True)
                try:
                    e.add_peer(BROADCAST_MAC, channel=ESP_NOW_CHANNEL)
                except TypeError:
                    e.add_peer(BROADCAST_MAC)
                return True
            except Exception as exc:
                e = None
                if report_errors:
                    print("ESP-NOW reset failed:", exc)
                    send_label.set_text("ESP-NOW reset failed")
                return False

        if not reset_espnow(False):
            status.set_text("ESP-NOW unavailable\ncontrols active")

        selected_rover_index = 0
        rover_selected = False
        axes = {"forward": CENTER, "steer": CENTER}
        axis_expires = {"forward": 0, "steer": 0}
        button_expires = {"a": 0, "b": 0, "x": 0, "s": 0}
        buttons = {"a": 0, "b": 0, "x": 0, "s": 0}
        send_count = 0
        tx_count = 0
        fail_count = 0

        def selected_rover_id():
            return ROVER_IDS[selected_rover_index]

        def show_control_widgets():
            grid.add_flag(lv.obj.FLAG.HIDDEN)
            instruction.add_flag(lv.obj.FLAG.HIDDEN)
            status.align_to(title, lv.ALIGN.OUT_BOTTOM_MID, 0, 10)
            key_label.align_to(status, lv.ALIGN.OUT_BOTTOM_MID, 0, 4)
            send_label.align_to(key_label, lv.ALIGN.OUT_BOTTOM_MID, 0, 4)
            mac_label.align_to(send_label, lv.ALIGN.OUT_BOTTOM_MID, 0, 4)
            help_box.align(lv.ALIGN.BOTTOM_MID, 0, -8)
            for widget in debug_widgets:
                widget.remove_flag(lv.obj.FLAG.HIDDEN)
            lv.group_focus_obj(screen)

        def start_control():
            nonlocal rover_selected
            rover_selected = True
            show_control_widgets()
            send_state(True)

        def choose_rover(index):
            nonlocal selected_rover_index
            if rover_selected:
                return
            selected_rover_index = index
            start_control()

        def make_packet():
            return struct.pack(
                "<hhBBBBB",
                axes["forward"],
                axes["steer"],
                buttons["a"],
                buttons["b"],
                buttons["x"],
                buttons["s"],
                ord(selected_rover_id()),
            )

        def describe_state():
            return "Rover %s F=%d S=%d\nshoot=%d dbl=%d up=%d down=%d" % (
                selected_rover_id(),
                axes["forward"],
                axes["steer"],
                buttons["a"],
                buttons["s"],
                buttons["x"],
                buttons["b"],
            )

        def send_state(force=False):
            nonlocal send_count, tx_count, fail_count
            if not rover_selected:
                return
            packet = make_packet()
            if e is None and not reset_espnow():
                fail_count += 1
                send_label.set_text("sent: %d tx: %d fail: %d" % (send_count, tx_count, fail_count))
                status.set_text(describe_state())
                return
            try:
                e.send(BROADCAST_MAC, packet)
            except Exception as exc:
                fail_count += 1
                print("ESP-NOW send failed:", exc)
                reset_espnow()
                send_label.set_text("sent: %d tx: %d fail: %d" % (send_count, tx_count, fail_count))
                status.set_text(describe_state())
                return
            tx_count += 1
            send_count += 1
            send_label.set_text("sent: %d tx: %d fail: %d" % (send_count, tx_count, fail_count))
            status.set_text(describe_state())

        def update_buttons():
            now = ticks_ms()
            expander_digital = read_expander_digital()
            for name in buttons:
                held_by_key = ticks_diff(button_expires[name], now) > 0
                expander_index = EXPANDER_BUTTONS.get(name)
                held_by_expander = False
                if (
                    expander_index is not None and
                    expander_digital is not None and
                    expander_index < len(expander_digital)
                ):
                    held_by_expander = bool(expander_digital[expander_index])
                buttons[name] = 1 if held_by_key or held_by_expander else 0

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
            if not rover_selected:
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

            send_state(True)

        use_flex = hasattr(grid, "set_flex_flow")
        for i, rover_letter in enumerate(ROVER_IDS):
            btn = lv.button(grid)
            btn.set_size(LETTER_BTN_W, LETTER_BTN_H)
            caption = lv.label(btn)
            caption.set_text(rover_letter)
            caption.center()
            btn.add_event_cb(
                lambda _event, rover_index=i: choose_rover(rover_index),
                lv.EVENT.CLICKED,
                None,
            )
            try:
                lv.group_remove_obj(btn)
            except Exception:
                pass
            if not use_flex:
                col = i % GRID_COLUMNS
                row = i // GRID_COLUMNS
                btn.set_pos(8 + col * (LETTER_BTN_W + 8), 4 + row * (LETTER_BTN_H + 8))

        lv.group_get_default().add_obj(screen)
        lv.group_focus_obj(screen)
        screen.add_event_cb(on_key, lv.EVENT.KEY, None)
        lv.timer_create(heartbeat, SEND_INTERVAL_MS, None)

        self.setContentView(screen)
