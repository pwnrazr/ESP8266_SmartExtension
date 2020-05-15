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
#include "Adafruit_MCP23017.h"

#include "settings.h"
#include "comms.h"
#include "OTAfunc.h"

Adafruit_MCP23017 mcp;

void setup() 
{
  Serial.begin(115200);
  commssetup();
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    Serial.print(".");
  }
  otasetup();
  mcp.begin();
}

void loop() 
{
  ArduinoOTA.handle();
}
