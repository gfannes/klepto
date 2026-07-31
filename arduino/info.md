 ## Getting the badge operational

 ### Flash latest image on it

Install esptool (necessary for flashing)
```
 python3 -m venv env
 source env/bin/activate.sh
 pip3 install esptool
```

* Download latest firmware from https://github.com/Fri3dCamp/badge_firmware/releases/tag/v1.0.1
  * 2024 badge: full_firmware_fox.img
  * 2022? badge: full_firmware_octopus.img
* Hook-up badge with USB-C cable and turn on
* Find out USB serial port
* flash with `esptool -p <port> --before default_reset --after no_reset --chip esp32s3 write_flash --flash_mode dio --flash_size 16MB --flash_freq 80m 0x0 <firmware.img>

### Update arduino IDE with correct board support

* Open preferences
* Press button right to Additional board manager
* Add the following URLS:
  * `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json` (for ESP chip)
  * `https://github.com/Fri3dCamp/badge_2024_arduino/releases/latest/download/package_fri3d-esp32_index.json` (for 2024 badge)
* Open Board Manager (Ctr; + Shift + B)
* Look for fri3d-esp32
* Install


### Install servo motor support

* Tools -> Manage libraries
* Search for ESP32
  * Library by Kevin Harrington & John K Bennett
* Install
  
### Get the ESP Now mac address

#### Sender side

* Plugin Badge
* Use Arduino to flash source onto device
  * /dev/ttyACM0 on my PC
  * Board is `Fri3d Badge 2024`
  * Upload
  * Select MycroPython 
    * Move with Y key
    * Select wih A key

It will print to serial something like `Sender MAC: 34:85:18:AC:37:B8`


#### Receiver side

* Plugin ESP32 on Rover
  * (!) Ensure no battery connected
* Use Arduino to flash onto device
  * /dev/ttyUSB port
  * Board is ESP32 Dev Module
  * Upload
  
It will print to serial something like: `Receiver MAC: 08:D1:F9:CB:F9:CC`






