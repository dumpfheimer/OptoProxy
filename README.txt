ABOUT THIS PROJECT

This project runs on ESP8266 (and probably is easily portable to ESP32) and connects to WiFi and Viessmann Heaters using Optolink (https://github.com/openv/openv/wiki/Die-Optolink-Schnittstelle) with a self-made adapter (https://github.com/openv/openv/wiki/Bauanleitung)
If you have a 3D printer check out OptolinkConnector.blend and OptolinkConnector.stl located in the hardware folder.

INSTALL INSTRUCTIONS (WIP)

1. Check out this repository

2. Download and install PlatformIO
	https://docs.platformio.org/en/latest/core/installation/index.html

3. Go to the software folder of this repository
    # e.g.
	$ cd software

4. copy wifi-credentials.h.example to wifi-credentials.h
    $ [ -f src/wifi-credentials.h ] || cp src/wifi-credentials.h.example src/wifi-credentials.h

5. edit the wifi credentials using your favourite text editor
    # e.g.
    $ vim src/wifi-credentials.h

6. Compile
    $ pio run

7. Upload to device
    $ pio run -t upload

8. Check log output
    $ pio device monitor

8. Open an issue with the error and hope for help, because things never work out the way you expect it
    https://github.com/dumpfheimer/OptoProxy/issues
