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
#include <Adafruit_MCP23X17.h>

#include "settings.h"

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

Adafruit_MCP23X17 mcp;

/*
byte buttonState0 = 0;
byte lastButtonState0 = 0;  

byte buttonState1 = 0;
byte lastButtonState1 = 0;  

byte buttonState2 = 0;
byte lastButtonState2 = 0;  
*/

byte buttonState3 = 0;
byte lastButtonState3 = 0;  

unsigned long switch_prevMillis = 0, heartbeat_prevMillis = 0, currentMillis;

#define switchInterval 200
#define heartbeatInterval 15000

#define MQTT_QOS 0

void setup() 
{
  WiFi.mode(WIFI_STA);
  commssetup();
  otasetup();
  mcp.begin_I2C();

  mcp.pinMode(0, OUTPUT);
  mcp.pinMode(1, OUTPUT);
  mcp.pinMode(2, OUTPUT);
  mcp.pinMode(3, OUTPUT);

  /*
  mcp.pinMode(4, INPUT_PULLUP);
  
  mcp.pinMode(5, INPUT_PULLUP);
  
  mcp.pinMode(6, INPUT_PULLUP);
  */
  
  mcp.pinMode(7, INPUT_PULLUP);

  // Properly set last button states
  /*
  lastButtonState0 = mcp.digitalRead(4);
  lastButtonState1 = mcp.digitalRead(5);
  lastButtonState2 = mcp.digitalRead(6);
  */
  lastButtonState3 = mcp.digitalRead(7);
}

void loop() 
{
  ArduinoOTA.handle();
  
  currentMillis = millis();

  if (currentMillis - switch_prevMillis >= switchInterval) 
  {
    switch_prevMillis = currentMillis;
    switchpolling();
  }

  if (currentMillis - heartbeat_prevMillis >= heartbeatInterval) 
  {
    heartbeat_prevMillis = currentMillis;
    mqttClient.publish("noderelay/heartbeat", MQTT_QOS, false, "Hi"); //publish to topic
  }
  
  if(currentMillis > 4094967296)  //overflow protection
  {
    ESP.restart();
  }
}

void switchpolling()
{
  /*
  //START Relay 0 Switch
  buttonState0 = mcp.digitalRead(4);
  if (buttonState0 != lastButtonState0) //Relay 0
  {
    setRelay(0, buttonState0);
  }
  lastButtonState0 = buttonState0;
  //END Relay 0 Switch

  //START Relay 1 Switch
  buttonState1 = mcp.digitalRead(5);
  if (buttonState1 != lastButtonState1) //Relay 1
  {
    setRelay(1, buttonState1);
  }
  lastButtonState1 = buttonState1;
  //END Relay 1 Switch

  //START Relay 2 Switch
  buttonState2 = mcp.digitalRead(6);
  if (buttonState2 != lastButtonState2) //Relay 2
  {
    setRelay(2, buttonState2);
  }
  lastButtonState2 = buttonState2;
  //END Relay 2 Switch
  */
  //START Relay 3 Switch
  buttonState3 = mcp.digitalRead(7);
  if (buttonState3 != lastButtonState3) //Relay 3
  {
    setRelay(0, buttonState3);
    setRelay(1, buttonState3);
    setRelay(2, buttonState3);
    setRelay(3, buttonState3);
  }
  lastButtonState3 = buttonState3;
  //END Relay 3 Switch
}

void otasetup() 
{
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
  }
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
  mqttClient.subscribe("noderelay/reqstat", MQTT_QOS);
  
  mqttClient.publish("noderelay/boot", MQTT_QOS, false, "0"); //publish to topic on boot
  char ipaddr[16];
  sprintf(ipaddr, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );

  mqttClient.publish("noderelay/ip", MQTT_QOS, false, ipaddr);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  //Serial.println("Publish received.");
  //Serial.println(topic);
  
  String payloadstr;
  int payloadint;
  
  for (int i = 0; i < len; i++) 
  {
    payloadstr = String(payloadstr + (char)payload[i]);  //convert payload to string
  }
  //Serial.println(payloadstr);

  payloadint = atoi(payload);  // Convert payload to integer
  
  //Relay 0
  if(strcmp((char*)topic, "noderelay/0") == 0)
  {
    setRelay(0, !payloadint);
  }

  //Relay 1
  if(strcmp((char*)topic, "noderelay/1") == 0)
  {
    setRelay(1, !payloadint);
  }

  //Relay 2
  if(strcmp((char*)topic, "noderelay/2") == 0)
  {
    setRelay(2, !payloadint);
  }

  //Relay 3
  if(strcmp((char*)topic, "noderelay/3") == 0)
  {
    setRelay(3, !payloadint);
  }

  if(strcmp((char*)topic, "noderelay/reboot") == 0)
  {
    ESP.restart();
  }

  if(strcmp((char*)topic, "noderelay/reqstat") == 0)  // Request statistics function
  {
    unsigned long REQ_STAT_CUR_MILLIS = millis(); // gets current millis

    char REQ_STAT_CUR_TEMPCHAR[60];

    snprintf(
      REQ_STAT_CUR_TEMPCHAR,
      60, 
      "%d.%d.%d.%d,%lu", 
      WiFi.localIP()[0], 
      WiFi.localIP()[1],
      WiFi.localIP()[2], 
      WiFi.localIP()[3],
      (int)REQ_STAT_CUR_MILLIS
    );  // convert string to char array for Millis. Elegance courtesy of Shahmi Technosparks

    mqttClient.publish("noderelay/curstat", MQTT_QOS, false, REQ_STAT_CUR_TEMPCHAR);
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

void setRelay(int relay, int state) 
{ // Note: relays are active LOW
  char charState[2];
  char charRelay[2];
  char relState[23] = "noderelay/relayState";
  
  itoa(!state, charState, 10);    // Invert charState from state as relay is active LOW, prevents Blynk seeing ON as OFF
  itoa(relay, charRelay, 10);
  
  strcat(relState, charRelay);    // Append charRelay to relState
  
  mcp.digitalWrite(relay, state);
  mqttClient.publish(relState, MQTT_QOS, false, charState); //publish to topic
}
