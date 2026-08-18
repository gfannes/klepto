# Klepto Rover (MicroPythonOS)

Badge controller for the Klepto Mars rover. Package id `com.fannes.klepto` (launcher title: **Klepto Rover**).

Pick a rover letter (A–K), then drive and shoot over ESP-NOW (broadcast, channel 1). Back, X, or swipe leaves the app.

## Controls

- **Joystick:** move the rover
- **Y:** aim up
- **B:** aim down
- **A:** shoot once
- **S:** burst (3 shots)

## Files

```
klepto/mp/
├── MANIFEST.JSON
├── icon_64x64.png
└── main.py
```

The desktop MicroPythonOS binary has no Wi-Fi/`espnow`. The app still starts there so you can check the UI; radio only works on the badge.

## Run on desktop

From the repo root (needs in-repo `MicroPythonOS/` and its desktop binary; see the juggling README):

```bash
ln -sfn "$(pwd)/klepto/mp" MicroPythonOS/internal_filesystem/apps/com.fannes.klepto
bash MicroPythonOS/scripts/run_desktop.sh com.fannes.klepto
```

## Push to the badge (`mpremote`)

Install the tool once:

```bash
python3 -m pip install --user mpremote
```

Plug the badge in with a **data** USB cable. List ports:

```bash
python3 -m mpremote connect list
```

You want a `/dev/ttyACM*` (or similar) line, not only `/dev/ttyS*`. On Linux, your user usually needs to be in the `dialout` group.

Copy the app so the folder name matches the package id, then refresh and start it:

```bash
python3 -m mpremote cp -r klepto/mp/. :/apps/com.fannes.klepto/
python3 -m mpremote exec "from mpos import AppManager; AppManager.refresh_apps()"
python3 -m mpremote exec "from mpos import AppManager; AppManager.start_app('com.fannes.klepto')"
```

If several serial devices are present, pass the port explicitly:

```bash
python3 -m mpremote connect /dev/ttyACM0 cp -r klepto/mp/. :/apps/com.fannes.klepto/
```

Run these from the **repo root**. After a code change, copy `main.py` again (and `MANIFEST.JSON` if you edited it) and refresh apps.
