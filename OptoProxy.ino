
// define the communication standard. can be OptolinkP300 or OptolinkKP
// check https://github.com/bertmelis/VitoWiFi for more information

#define OPTOLINK_CLASS OptolinkP300

#include <OptolinkP300.hpp>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>


// pin 2->D9
// pin 0->D8
#define OPTOLINK_OUT 0//D8
#define OPTOLINK_IN 2 //D9


ESP8266WebServer server(80);

const char* kSsid = "CHANGEME";
const char* kPassword = "CHANGEME";

OPTOLINK_CLASS optolink;

void setupLogging();
void setupHttp();
//void println(String s);
//bool useLogging();
//SoftwareSerial* getLogger();

void setup() {
  setupLogging();
  println("hello! logging started");

  WiFi.begin(kSsid, kPassword);

  setupHttp();

  optolink.begin(&Serial);
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
