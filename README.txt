ABOUT THIS PROJECT

This project runs on ESP8266 (and probably is easily portable to ESP32) and connects to WiFi and Viessmann Heaters using Optolink (https://github.com/openv/openv/wiki/Die-Optolink-Schnittstelle) with a self-made adapter (https://github.com/openv/openv/wiki/Bauanleitung)
If you have a 3D printer check out OptolinkConnector.blend and OptolinkConnector.stl

INSTALL INSTRUCTIONS (WIP)

1. Download and install Arduino IDE
	https://www.arduino.cc/en/Guide

2. Download and install ESP8266 
	https://github.com/esp8266/Arduino

3. Download and install VitoWifi library
	https://github.com/bertmelis/VitoWiFi/
	check it out in your libraries folder or download the zip and unpack it (for me it was ~/Arduino/libraries)

4. In the Arduino IDE select ESP8266 as you board

5. Add the VitoWifi Library to your project (not sure if you actually need to do that?!)

6. copy wifi-credentials.h.example to wifi-credentials.h and change the credentials to your desire

7. Hit compile

8. Open an issue with the error and hope for help
