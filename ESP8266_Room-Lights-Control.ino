/*
 * To setup credentials and other settings create a "settings.h" file in root of this sketch with this code below:
  #define WIFI_SSID "My_Wi-Fi"
  #define WIFI_PASSWORD "password"
  
  #define MQTT_HOST IPAddress(192, 168, 1, 10)
  #define MQTT_PORT 1883
  #define MQTT_USER "mqttUSR"
  #define MQTT_PASS "mqttPASS"

  #define OTA_PASS "otaPASS"
 */
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <ArduinoOTA.h>
#include "Adafruit_MCP23017.h"

#include "settings.h"
#include "comms.h"
#include "OTAfunc.h"

Adafruit_MCP23017 mcp;

int buttonState0 = 0;
int lastButtonState0 = 0;  

int buttonState1 = 0;
int lastButtonState1 = 0;  

int buttonState2 = 0;
int lastButtonState2 = 0;  

int buttonState3 = 0;
int lastButtonState3 = 0;  

int lastval0 = 0;
int lastval1 = 0;
int lastval2 = 0;
int lastval3 = 0;

char charval0[1];
char charval1[1];
char charval2[1];
char charval3[1];

unsigned long previousMillis = 0, previousMillis2 = 0, currentMillis;
long interval = 200, interval2 = 15000;

void setup() 
{
  Serial.begin(115200);
  commssetup();
  otasetup();
  mcp.begin();

  mcp.pinMode(0, OUTPUT);
  mcp.pinMode(1, OUTPUT);
  mcp.pinMode(2, OUTPUT);
  mcp.pinMode(3, OUTPUT);
  
  mcp.pinMode(4, INPUT);
  mcp.pullUp(4, HIGH);  // turn on a 100K pullup internally
  
  mcp.pinMode(5, INPUT);
  mcp.pullUp(5, HIGH);  // turn on a 100K pullup internally
  
  mcp.pinMode(6, INPUT);
  mcp.pullUp(6, HIGH);  // turn on a 100K pullup internally
  
  mcp.pinMode(7, INPUT);
  mcp.pullUp(7, HIGH);  // turn on a 100K pullup internally
}

void loop() 
{
  ArduinoOTA.handle();
}
