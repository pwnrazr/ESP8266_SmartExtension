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

#include "settings.h"

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

byte buttonState0 = 0;
byte lastButtonState0 = 0;  

unsigned long switch_prevMillis = 0, heartbeat_prevMillis = 0;

#define switchInterval 200
#define heartbeatInterval 15000

#define MQTT_QOS 0

#define manualSwitch 5  // Input01 GPIO5/D1 (For manual on/off)

#define relayCh01 4     // Channel1 GPIO4/D2
#define relayCh02 14    // Channel2 GPIO14/D5
#define relayCh03 12    // Channel3 GPIO12/D6
#define relayCh04 13    // Channel4 GPIO13/D7

void setup() 
{
  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  commssetup();
  otasetup();
  
  pinMode(manualSwitch, INPUT);
  
  pinMode(relayCh01, OUTPUT);
  digitalWrite(relayCh01, HIGH); // Turn off relay on boot
  
  pinMode(relayCh02, OUTPUT);
  digitalWrite(relayCh02, HIGH); // Turn off relay on boot
  
  pinMode(relayCh03, OUTPUT);
  digitalWrite(relayCh03, HIGH); // Turn off relay on boot
  
  pinMode(relayCh04, OUTPUT);
  digitalWrite(relayCh04, HIGH); // Turn off relay on boot
  
  // Properly set last button states
  lastButtonState0 = digitalRead(manualSwitch);
}

void loop() 
{
  ArduinoOTA.handle();
  
  if (millis() - switch_prevMillis >= switchInterval) 
  {
    switch_prevMillis = millis();
    switchpolling();
  }

  if (millis() - heartbeat_prevMillis >= heartbeatInterval) 
  {
    heartbeat_prevMillis = millis();
    mqttClient.publish("noderelay/heartbeat", MQTT_QOS, false, "Hi"); //publish to topic
  }
}

void switchpolling()
{
  buttonState0 = digitalRead(manualSwitch);
  if (buttonState0 != lastButtonState0) // Manual Switch
  {
    if(
      digitalRead(relayCh01) == LOW ||
      digitalRead(relayCh02) == LOW ||
      digitalRead(relayCh03) == LOW ||
      digitalRead(relayCh04) == LOW)
      {
        setRelay(relayCh01, HIGH);    // Turn all off only if any of the sockets are turned on
        setRelay(relayCh02, HIGH);
        setRelay(relayCh03, HIGH);
        setRelay(relayCh04, HIGH);
      }
      else
      {
        setRelay(relayCh01, LOW);     // Turn all on when none are on
        setRelay(relayCh02, LOW); 
        setRelay(relayCh03, LOW);
        setRelay(relayCh04, LOW);
      }
  }
  lastButtonState0 = buttonState0;
}

void otasetup() 
{
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("NodeRelay");

  // No authentication by default
  ArduinoOTA.setPassword(OTA_PASS);
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
    
    //Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    //Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    //Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      //Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      //Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      //Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      //Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  //Serial.println("OTA Ready");
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());
}

void connectToWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  mqttClient.connect();
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void onMqttConnect(bool sessionPresent) 
{
  mqttClient.subscribe("noderelay/0", MQTT_QOS);
  mqttClient.subscribe("noderelay/1", MQTT_QOS);
  mqttClient.subscribe("noderelay/2", MQTT_QOS);
  mqttClient.subscribe("noderelay/3", MQTT_QOS);
  mqttClient.subscribe("noderelay/reboot", MQTT_QOS);
  mqttClient.subscribe("noderelay/request_uptime", MQTT_QOS);
  mqttClient.subscribe("noderelay/sync", MQTT_QOS);
  
  char ipaddr[16];
  snprintf(ipaddr, 16, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );

  mqttClient.publish("noderelay/ip", MQTT_QOS, false, ipaddr);
  mqttClient.publish("noderelay/relayState0", MQTT_QOS, false, "0");
  mqttClient.publish("noderelay/relayState1", MQTT_QOS, false, "0");
  mqttClient.publish("noderelay/relayState2", MQTT_QOS, false, "0");
  mqttClient.publish("noderelay/relayState3", MQTT_QOS, false, "0");
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  //Serial.println("Publish received.");
  //Serial.println(topic);
  
  byte payloadint;
  
  payloadint = atoi(payload);  // Convert payload to integer
  
  //Relay 0
  if(strcmp((char*)topic, "noderelay/0") == 0)
  {
    setRelay(relayCh01, !payloadint);
  }

  //Relay 1
  if(strcmp((char*)topic, "noderelay/1") == 0)
  {
    setRelay(relayCh02, !payloadint);
  }

  //Relay 2
  if(strcmp((char*)topic, "noderelay/2") == 0)
  {
    setRelay(relayCh03, !payloadint);
  }

  //Relay 3
  if(strcmp((char*)topic, "noderelay/3") == 0)
  {
    setRelay(relayCh04, !payloadint);
  }

  if(strcmp((char*)topic, "noderelay/reboot") == 0)
  {
    ESP.restart();
  }

  if(strcmp((char*)topic, "noderelay/sync") == 0)
  {
    byte relayCh01State = !digitalRead(relayCh01);
    byte relayCh02State = !digitalRead(relayCh02);
    byte relayCh03State = !digitalRead(relayCh03);
    byte relayCh04State = !digitalRead(relayCh04);

    char relayCh01StateChar[2];
    char relayCh02StateChar[2];
    char relayCh03StateChar[2];
    char relayCh04StateChar[2];

    itoa(relayCh01State, relayCh01StateChar, 10);
    itoa(relayCh02State, relayCh02StateChar, 10);
    itoa(relayCh03State, relayCh03StateChar, 10);
    itoa(relayCh04State, relayCh04StateChar, 10);
    
    mqttClient.publish("noderelay/relayState0", MQTT_QOS, false, relayCh01StateChar);
    mqttClient.publish("noderelay/relayState1", MQTT_QOS, false, relayCh02StateChar);
    mqttClient.publish("noderelay/relayState2", MQTT_QOS, false, relayCh03StateChar);
    mqttClient.publish("noderelay/relayState3", MQTT_QOS, false, relayCh04StateChar);
  }
  
  if(strcmp((char*)topic, "noderelay/request_uptime") == 0)  // Request statistics function
  {
    unsigned long CUR_MILLIS = millis() / 1000; // Gets current millis and divide by 1000 to get in seconds
    
    char BUFFER_CUR_TEMPCHAR[30];
    sprintf(BUFFER_CUR_TEMPCHAR, "%lu", CUR_MILLIS);
    
    mqttClient.publish("noderelay/uptime", MQTT_QOS, false, BUFFER_CUR_TEMPCHAR);
  }
}

void commssetup() {
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(MQTT_USER, MQTT_PASS);
  connectToWifi();
}

void setRelay(byte relay, byte state) 
{ // Note: relays are active LOW
  char charState[2];
  char charRelay[2];
  char relState[23] = "noderelay/relayState";
  
  digitalWrite(relay, state);
  
  switch(relay)
  {
    case relayCh01:
      relay = 0;
      break;
    case relayCh02:
      relay = 1;
      break;
    case relayCh03:
      relay = 2;
      break;
    case relayCh04:
      relay = 3;
      break;
  }
  
  itoa(!state, charState, 10);    // Invert charState from state as relay is active LOW
  itoa(relay, charRelay, 10);
  
  strcat(relState, charRelay);    // Append charRelay to relState
  
  mqttClient.publish(relState, MQTT_QOS, false, charState); //publish to topic
}
