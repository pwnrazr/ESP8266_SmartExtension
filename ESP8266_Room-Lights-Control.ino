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

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

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

void otasetup() 
{
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    Serial.print(".");
  }
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("NodeRelay_DEV");

  // No authentication by default
  ArduinoOTA.setPassword(OTA_PASS);

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("OTA Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.println("Subscribing...");
  //mqttClient.subscribe("test/lol", 2);
  mqttClient.subscribe("/myroom/relay/0", 2);
  mqttClient.subscribe("/myroom/relay/1", 2);
  mqttClient.subscribe("/myroom/relay/2", 2);
  mqttClient.subscribe("/myroom/relay/3", 2);
  mqttClient.subscribe("/node_relay/reboot", 2);
  mqttClient.subscribe("/node_relay/reqstat", 2);
  
  mqttClient.publish("/myroom/relay/boot", 2, false, "0"); //publish to topic on boot
  char ipaddr[16];
  sprintf(ipaddr, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );
  //mqttClient.publish("test/lol", 1, true, "test 2");
  mqttClient.publish("/myroom/relay/ip", 2, false, ipaddr);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  
  Serial.println(total);

  String payloadstr;

  for (int i = 0; i < len; i++) 
  {
    payloadstr = String(payloadstr + (char)payload[i]);  //convert payload to string
  }

  Serial.println(payloadstr);
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void commssetup() {
  Serial.println();
  Serial.println();

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(MQTT_USER, MQTT_PASS);
  connectToWifi();
}
