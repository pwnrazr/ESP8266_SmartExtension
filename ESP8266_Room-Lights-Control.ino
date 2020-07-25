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

//web server
WiFiServer server(80);

byte buttonState0 = 0;
byte lastButtonState0 = 0;  

byte buttonState1 = 0;
byte lastButtonState1 = 0;  

byte buttonState2 = 0;
byte lastButtonState2 = 0;  

byte buttonState3 = 0;
byte lastButtonState3 = 0;  

byte lastval0 = 0;
byte lastval1 = 0;
byte lastval2 = 0;
byte lastval3 = 0;

char charval0[1];
char charval1[1];
char charval2[1];
char charval3[1];

unsigned long switch_prevMillis = 0, heartbeat_prevMillis = 0, currentMillis;

#define switchInterval 200
#define heartbeatInterval 15000

#define MQTT_QOS 0

void setup() 
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
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
  
  currentMillis = millis();
  
  webServerHandle();
    
  if (currentMillis - switch_prevMillis >= switchInterval) 
  {
    switch_prevMillis = currentMillis;
    switchpolling();
  }

  if (currentMillis - heartbeat_prevMillis >= heartbeatInterval) 
  {
    heartbeat_prevMillis = currentMillis;
    mqttClient.publish("/nodeRelay/heartbeat", MQTT_QOS, false, "Hi"); //publish to topic
  }
  
  if(currentMillis > 4094967296)  //overflow protection
  {
    ESP.restart();
  }
}

void switchpolling()
{
  //START Relay 0 Switch
  buttonState0 = mcp.digitalRead(4);
  if (buttonState0 != lastButtonState0) //Relay 0
  {
    Serial.println("Changed STATE Relay 0");
    mcp.digitalWrite(0, !lastval0);
    itoa(lastval0, charval0, 10);
    lastval0 = !lastval0;
    Serial.println(lastval0);
    mqttClient.publish("/myroom/relay/relState0", MQTT_QOS, false, charval0); //publish to topic
  }
  lastButtonState0 = buttonState0;
  //END Relay 0 Switch

  //START Relay 1 Switch
  buttonState1 = mcp.digitalRead(5);
  if (buttonState1 != lastButtonState1) //Relay 1
  {
    Serial.println("Changed STATE Relay 1");
    mcp.digitalWrite(1, !lastval1);
    itoa(lastval1, charval1, 10);
    lastval1 = !lastval1;
    Serial.println(lastval1);
    mqttClient.publish("/myroom/relay/relState1", MQTT_QOS, false, charval1); //publish to topic
  }
  lastButtonState1 = buttonState1;
  //END Relay 1 Switch

  //START Relay 2 Switch
  buttonState2 = mcp.digitalRead(6);
  if (buttonState2 != lastButtonState2) //Relay 2
  {
    Serial.println("Changed STATE Relay 2");
    mcp.digitalWrite(2, !lastval2);
    itoa(lastval2, charval2, 10);
    lastval2 = !lastval2;
    Serial.println(lastval2);
    mqttClient.publish("/myroom/relay/relState2", MQTT_QOS, false, charval2); //publish to topic
  }
  lastButtonState2 = buttonState2;
  //END Relay 2 Switch

  //START Relay 3 Switch
  buttonState3 = mcp.digitalRead(7);
  if (buttonState3 != lastButtonState3) //Relay 3
  {
    Serial.println("Changed STATE Relay 3");
    mcp.digitalWrite(3, !lastval3);
    itoa(lastval3, charval3, 10);
    lastval3 = !lastval3;
    Serial.println(lastval3);
    mqttClient.publish("/myroom/relay/relState3", MQTT_QOS, false, charval3); //publish to topic
  }
  lastButtonState3 = buttonState3;
  //END Relay 3 Switch
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
  ArduinoOTA.setHostname("NodeRelay");

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
  server.begin();
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
  mqttClient.subscribe("/myroom/relay/0", MQTT_QOS);
  mqttClient.subscribe("/myroom/relay/1", MQTT_QOS);
  mqttClient.subscribe("/myroom/relay/2", MQTT_QOS);
  mqttClient.subscribe("/myroom/relay/3", MQTT_QOS);
  mqttClient.subscribe("/node_relay/reboot", MQTT_QOS);
  mqttClient.subscribe("/node_relay/reqstat", MQTT_QOS);
  
  mqttClient.publish("/myroom/relay/boot", MQTT_QOS, false, "0"); //publish to topic on boot
  char ipaddr[16];
  sprintf(ipaddr, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );
  //mqttClient.publish("test/lol", 1, true, "test 2");
  mqttClient.publish("/myroom/relay/ip", MQTT_QOS, false, ipaddr);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
}

void onMqttUnsubscribe(uint16_t packetId) {
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.println(topic);
  
  String payloadstr;

  for (int i = 0; i < len; i++) 
  {
    payloadstr = String(payloadstr + (char)payload[i]);  //convert payload to string
  }
  Serial.println(payloadstr);

  //Relay 0
  if(strcmp((char*)topic, "/myroom/relay/0") == 0)
  {
    if(payloadstr=="0") 
    {
      mcp.digitalWrite(0, HIGH);
      Serial.println("Relay 0 OFF");
      lastval0 = 1;
      Serial.println(lastval0);
      mqttClient.publish("/myroom/relay/relState0", MQTT_QOS, false, "0"); //publish to topic
    }
    else if(payloadstr=="1")
    {
      mcp.digitalWrite(0, LOW);
      Serial.println("Relay 0 ON"); 
      lastval0 = 0;
      Serial.println(lastval0);
      mqttClient.publish("/myroom/relay/relState0", MQTT_QOS, false, "1"); //publish to topic
    }
  }

  //Relay 1
  if(strcmp((char*)topic, "/myroom/relay/1") == 0)
  {
    if(payloadstr=="0") 
    {
      mcp.digitalWrite(1, HIGH);
      Serial.println("Relay 1 OFF");
      lastval1 = 1;
      Serial.println(lastval1);
      mqttClient.publish("/myroom/relay/relState1", MQTT_QOS, false, "0"); //publish to topic
    }
    else if(payloadstr=="1")
    {
      mcp.digitalWrite(1, LOW);
      Serial.println("Relay 1 ON"); 
      lastval1 = 0;
      Serial.println(lastval1);
      mqttClient.publish("/myroom/relay/relState1", MQTT_QOS, false, "1"); //publish to topic
    }
  }

  //Relay 2
  if(strcmp((char*)topic, "/myroom/relay/2") == 0)
  {
    if(payloadstr=="0") 
    {
      mcp.digitalWrite(2, HIGH);
      Serial.println("Relay 2 OFF");
      lastval2 = 1;
      Serial.println(lastval2);
      mqttClient.publish("/myroom/relay/relState2", MQTT_QOS, false, "0"); //publish to topic
    }
    else if(payloadstr=="1")
    {
      mcp.digitalWrite(2, LOW);
      Serial.println("Relay 2 ON"); 
      lastval2 = 0;
      Serial.println(lastval2);
      mqttClient.publish("/myroom/relay/relState2", MQTT_QOS, false, "1"); //publish to topic
    }
  }

  //Relay 3
  if(strcmp((char*)topic, "/myroom/relay/3") == 0)
  {
    if(payloadstr=="0") 
    {
      mcp.digitalWrite(3, HIGH);
      Serial.println("Relay 3 OFF");
      lastval3 = 1;
      Serial.println(lastval3);
      mqttClient.publish("/myroom/relay/relState3", MQTT_QOS, false, "0"); //publish to topic
    }
    else if(payloadstr=="1")
    {
      mcp.digitalWrite(3, LOW);
      Serial.println("Relay 3 ON"); 
      lastval3 = 0;
      Serial.println(lastval3);
      mqttClient.publish("/myroom/relay/relState3", MQTT_QOS, false, "1"); //publish to topic
    }
  }

  if(strcmp((char*)topic, "/node_relay/reboot") == 0)
  {
    ESP.restart();
  }

  if(strcmp((char*)topic, "/node_relay/reqstat") == 0)  // Request statistics function
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

    mqttClient.publish("/node_relay/curstat", MQTT_QOS, false, REQ_STAT_CUR_TEMPCHAR);
  }
}

void onMqttPublish(uint16_t packetId) {
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

void webServerHandle(){
  //web server begin
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) { // If there is a client...
    boolean currentLineIsBlank = true;
    String buffer = ""; // A buffer for the GET request
    
    while (client.connected()) {
  
      if (client.available()) {
        char c = client.read();// Read the data of the client
        buffer += c; // Store the data in a buffer
        
        if (c == '\n' && currentLineIsBlank) {// if 2x new line ==> Request ended
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println(); // Blank line ==> end response
          mqttClient.publish("node_relay/webservTest", 0, false, "Node Relay HTTP response");
          break;
        }
        if (c == '\n') { // if New line
          currentLineIsBlank = true;
          buffer = "";  // Clear buffer
        } else if (c == '\r') { // If cariage return...
          //Read in the buffer if there was send "GET /?..."
          
          if(buffer.indexOf("GET /?relay0=0")>=0) { // If relay0 OFF
            mcp.digitalWrite(0, HIGH);
            Serial.println("Relay 0 OFF");
            lastval0 = 1;
            Serial.println(lastval0);
            mqttClient.publish("/myroom/relay/relState0", MQTT_QOS, false, "0"); //publish to topic
            client.println("HTTP/1.1 200 ack,ack");
          }
          if(buffer.indexOf("GET /?relay0=1")>=0) { // If relay0 ON
            mcp.digitalWrite(0, LOW);
            Serial.println("Relay 0 ON"); 
            lastval0 = 0;
            Serial.println(lastval0);
            mqttClient.publish("/myroom/relay/relState0", MQTT_QOS, false, "1"); //publish to topic
            client.println("HTTP/1.1 200 ack,ack");
          }

          if(buffer.indexOf("GET /?relay1=0")>=0) { // If relay1 OFF
            mcp.digitalWrite(1, HIGH);
            Serial.println("Relay 1 OFF");
            lastval1 = 1;
            Serial.println(lastval1);
            mqttClient.publish("/myroom/relay/relState1", MQTT_QOS, false, "0"); //publish to topic
            client.println("HTTP/1.1 200 ack,ack");
          }
          if(buffer.indexOf("GET /?relay1=1")>=0) { // If relay1 ON
            mcp.digitalWrite(1, LOW);
            Serial.println("Relay 1 ON"); 
            lastval1 = 0;
            Serial.println(lastval1);
            mqttClient.publish("/myroom/relay/relState1", MQTT_QOS, false, "1"); //publish to topic
            client.println("HTTP/1.1 200 ack,ack");
          }
          
          if(buffer.indexOf("GET /?relay2=0")>=0) { // If relay2 OFF
            mcp.digitalWrite(2, HIGH);
            Serial.println("Relay 2 OFF");
            lastval2 = 1;
            Serial.println(lastval2);
            mqttClient.publish("/myroom/relay/relState2", MQTT_QOS, false, "0"); //publish to topic
            client.println("HTTP/1.1 200 ack,ack");
          }
          if(buffer.indexOf("GET /?relay2=1")>=0) { // If relay2 ON
            mcp.digitalWrite(2, LOW);
            Serial.println("Relay 2 ON"); 
            lastval2 = 0;
            Serial.println(lastval2);
            mqttClient.publish("/myroom/relay/relState2", MQTT_QOS, false, "1"); //publish to topic
            client.println("HTTP/1.1 200 ack,ack");
          }

          if(buffer.indexOf("GET /?relay3=0")>=0) { // If relay3 OFF
            mcp.digitalWrite(3, HIGH);
            Serial.println("Relay 3 OFF");
            lastval3 = 1;
            Serial.println(lastval3);
            mqttClient.publish("/myroom/relay/relState3", MQTT_QOS, false, "0"); //publish to topic
            client.println("HTTP/1.1 200 ack,ack");
          }
          if(buffer.indexOf("GET /?relay3=1")>=0) { // If relay3 ON
            mcp.digitalWrite(3, LOW);
            Serial.println("Relay 3 ON"); 
            lastval3 = 0;
            Serial.println(lastval3);
            mqttClient.publish("/myroom/relay/relState3", MQTT_QOS, false, "1"); //publish to topic
            client.println("HTTP/1.1 200 ack,ack");
          }
          /////////////////////////////
          if(buffer.indexOf("GET /?req_relay0")>=0) { // App request of relay0 status
            if(mcp.digitalRead(0)==LOW)
            {
              client.println("HTTP/1.1 200 relay0,on");
            }
            else{
              client.println("HTTP/1.1 200 relay0,off");
            }
          }
          if(buffer.indexOf("GET /?req_relay1")>=0) { // App request of relay1 status
            if(mcp.digitalRead(1)==LOW)
            {
              client.println("HTTP/1.1 200 relay1,on");
            }
            else{
              client.println("HTTP/1.1 200 relay1,off");
            }
          }
          if(buffer.indexOf("GET /?req_relay2")>=0) { // App request of relay2 status
            if(mcp.digitalRead(2)==LOW)
            {
              client.println("HTTP/1.1 200 relay2,on");
            }
            else{
              client.println("HTTP/1.1 200 relay2,off");
            }
          }
          if(buffer.indexOf("GET /?req_relay3")>=0) { // App request of relay3 status
            if(mcp.digitalRead(3)==LOW)
            {
              client.println("HTTP/1.1 200 relay3,on");
            }
            else{
              client.println("HTTP/1.1 200 relay3,off");
            }
          }
          
        } else {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
  }
  //web server end
}
