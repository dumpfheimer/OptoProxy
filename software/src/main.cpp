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
  WiFi.persistent(true);
  
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

  #if OPTOLINK_LOOP_THREADED == true
  xTaskCreatePinnedToCore(
                    optolinkLoop,   /* Task function. */
                    "optolinkTask",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &optolinkTaskHandle,      /* Task handle to keep track of created task */
                    OPTOLINK_LOOP_CORE);          /* pin task to core 0 */  
  #endif
  
  #if HTTPSERVER_LOOP_THREADED == true
  xTaskCreatePinnedToCore(
                    httpServerLoop,   /* Task function. */
                    "httpServerTask",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &httpserverTaskHandle,      /* Task handle to keep track of created task */
                    HTTPSERVER_LOOP_CORE);          /* pin task to core 0 */  
  #endif
}
OPTOLINK_CLASS* getOptolink() {
  return &optolink;
}
void optolinkLoop(void * pvParameters) {
  while(1) {
    optolink.loop();
  }
}
void httpServerLoop(void * pvParameters) {
  while(1) {
    server.handleClient();
  }
}
void loop() {
  #if OPTOLINK_LOOP_THREADED == false
  optolink.loop();
  #endif
  
  #if HTTPSERVER_LOOP_THREADED == false
  server.handleClient();
  #endif
}
