# ABOUT THIS PROJECT

This project runs on ESP8266 and ESP32 and connnects to WiFi and Viessmann Heaters using KM BUS (https://github.com/openv/openv/wiki/KM-Bus) over TTL<->M-BUS adapter (https://www.aliexpress.com/item/32751482255.html?spm=a2g0o.productlist.0.0.43744c91mj7fO8&algo_pvid=92261257-59d5-45e2-b1d3-575c4333a3b1&algo_expid=92261257-59d5-45e2-b1d3-575c4333a3b1-0&btsid=0bb0622a16021520273081710ef490&ws_ab_test=searchweb0_0,searchweb201602_,searchweb201603_)

# INSTALL INSTRUCTIONS

1. Check out this repository
2. Download and install PlatformIO
   https://docs.platformio.org/en/latest/core/installation/index.html
3. Go to the software folder of this repository

   $ cd software

4. Compile and Upload

   $ pio run -t upload

5. (optional) Check log output

   $ pio device monitor

6. Configure the device (if not done with configuration.h)
   The device, if unconfigured, will open a Hotspot (Portal-XXXXXXXXXX)
   Connect to the hotspot and navigate to http://192.168.4.1/wifiMgr/configure
   Enter SSID, Password, Hostname, MQTT Host Username and Password (optional)
   Click submit

7. Open an issue with the error and hope for help, because things never work out the way you expect it

    https://github.com/dumpfheimer/WiFiVitotrol/issues

# WIRING THE ESP8266
on the ESP8266 the default Serial (RX/TX) will be for logging and a SoftwareSerial is used for heater communication
The Adapter will be connected to GROUND / 3.3V / D4 (RX) / D5 (TX)

# WIRING THE ESP32
on the ESP32 the default Serial (RX/TX) will be for logging and Serial1 is used for heater communication.
The Adapter will be connected to GROUND / 3.3V / RX / TX of Serial1


# HTTP ENDPOINTS
/ will show you a status page with a lot of technical details
/getCurrentRoomTemperature returns the current room temperature set
/getDesiredRoomTemperature returns the desired room temperature set