#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "Adafruit_MCP23017.h"
#include "wifiConf.h"
#include "mqttfunc.h"
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
  pinMode(2, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Wire.begin(4, 5); //SDA, SCL
  
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
  
  //Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  OTAfunc();
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  String topicstr;
  String payloadstr;
  
  for (int i = 0; i < length; i++) 
  {
    payloadstr = String(payloadstr + (char)payload[i]);  //convert payload to string
  }
  
  for(int i = 0; i <= 50; i++)
  {
    topicstr = String(topicstr + (char)topic[i]);  //convert topic to string
  }

  Serial.print("Message arrived: ");
  Serial.print(topicstr);
  Serial.print(" - ");
  Serial.print(payloadstr);
  Serial.println();
  
  //start
  if(topicstr=="/myroom/relay/0")
  {
    if(payloadstr=="0") 
    {
      //digitalWrite(2, LOW); 
      mcp.digitalWrite(0, HIGH);
      Serial.println("Relay 0 OFF"); 
      lastval0 = 1;
      Serial.println(lastval0);
      client.publish("/myroom/relay/relState0", "0"); //publish to topic
    }
    else if(payloadstr=="1")
    {
      //digitalWrite(2, HIGH);
      mcp.digitalWrite(0, LOW);
      Serial.println("Relay 0 ON"); 
      lastval0 = 0;
      Serial.println(lastval0);
      client.publish("/myroom/relay/relState0", "1"); //publish to topic
    }
  }

  if(topicstr=="/myroom/relay/1")
  {
    if(payloadstr=="0") 
    {
      mcp.digitalWrite(1, HIGH);
      Serial.println("Relay 1 OFF"); 
      lastval1 = 1;
      Serial.println(lastval1);
      client.publish("/myroom/relay/relState1", "0"); //publish to topic
    }
    else if(payloadstr=="1")
    {
      mcp.digitalWrite(1, LOW);
      Serial.println("Relay 1 ON"); 
      lastval1 = 0;
      Serial.println(lastval1);
      client.publish("/myroom/relay/relState1", "1"); //publish to topic
    }
  }

  if(topicstr=="/myroom/relay/2")
  {
    if(payloadstr=="0") 
    {
      mcp.digitalWrite(2, HIGH);
      Serial.println("Relay 2 OFF"); 
      lastval2 = 1;
      Serial.println(lastval2);
      client.publish("/myroom/relay/relState2", "0"); //publish to topic
    }
    else if(payloadstr=="1")
    {
      mcp.digitalWrite(2, LOW);
      Serial.println("Relay 2 ON"); 
      lastval2 = 0;
      Serial.println(lastval2);
      client.publish("/myroom/relay/relState2", "1"); //publish to topic
    }
  }

  if(topicstr=="/myroom/relay/3")
  {
    if(payloadstr=="0") 
    {
      mcp.digitalWrite(3, HIGH);
      Serial.println("Relay 3 OFF"); 
      lastval3 = 1;
      Serial.println(lastval3);
      client.publish("/myroom/relay/relState3", "0"); //publish to topic
    }
    else if(payloadstr=="1")
    {
      mcp.digitalWrite(3, LOW);
      Serial.println("Relay 3 ON"); 
      lastval3 = 0;
      Serial.println(lastval3);
      client.publish("/myroom/relay/relState3", "1"); //publish to topic
    }
  }

  if(topicstr=="/node_relay/reboot")
  {
    client.publish("/node_relay/status", "reboot called");
    delay(1000);
    ESP.restart();
  }
}

void loop() 
{
  currentMillis = millis();
  
  if (!client.connected()) 
    {
      reconnect();
    }
  
  client.loop();
  ArduinoOTA.handle();

  relaypolling();

  if (currentMillis - previousMillis2 >= interval2) 
  {
    previousMillis2 = currentMillis;
    client.publish("/nodeRelay/heartbeat", "Hi");
  }
  
  if(currentMillis > 4094967296)
  {
    Serial.println("reboot");
    Serial.println(currentMillis);
    ESP.restart();
  }
}

void relaypolling()
{
  if (currentMillis - previousMillis >= interval) 
  {
    previousMillis = currentMillis;
    //START Relay 0 Switch
  buttonState0 = mcp.digitalRead(4);
  if (buttonState0 != lastButtonState0) //Relay 0
  {
    Serial.println("Changed STATE Relay 0");
    mcp.digitalWrite(0, !lastval0);
    itoa(lastval0, charval0, 10);
    lastval0 = !lastval0;
    Serial.println(lastval0);
    client.publish("/myroom/relay/relState0", charval0); //publish to topic
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
    client.publish("/myroom/relay/relState1", charval1); //publish to topic
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
    client.publish("/myroom/relay/relState2", charval2); //publish to topic
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
    client.publish("/myroom/relay/relState3", charval3); //publish to topic
  }
  lastButtonState3 = buttonState3;
  //END Relay 3 Switch 
  }
}
