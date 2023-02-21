#include "main.h"

#ifdef ESP8266
ESP8266WebServer server(80);
#endif
#ifdef ESP32
WebServer server(80);
#endif

OPTOLINK_CLASS optolink;

void setupLogging();
void setupHttp();

void setup() {
  setupLogging();
  println("hello! logging started");

  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  #ifdef ESP32
  WiFi.setHostname("OptoProxy");
  #endif

  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  
  setupHttp();
  ElegantOTA.begin(&server);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  if (!MDNS.begin("OptoProxy")) {
    println("Error setting up MDNS responder!");
  }


  #ifdef ESP8266
  optolink.begin(&Serial);
  #endif
  #ifdef ESP32
  optolink.begin(&Serial1, OPTOLINK_SERIAL_RX, OPTOLINK_SERIAL_TX);
  #endif

  if (useLogging()) {
    optolink.setLogger(getLogger());
  }
}
OPTOLINK_CLASS* getOptolink() {
  return &optolink;
}
void loop() {
  optolink.loop();
  server.handleClient();
}
