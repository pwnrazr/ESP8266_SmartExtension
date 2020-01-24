#include <PubSubClient.h>
#include <ESP8266WiFi.h>

WiFiClient espClient;
PubSubClient client(espClient);

const char* mqtt_server = "192.168.1.184";
const char* mqtt_username = "pwnrazr";
const char* mqtt_password = "pwnrazr123";

char ipaddr[16];

void subscribemqtt()
{
  client.subscribe("/myroom/relay/0");
  client.subscribe("/myroom/relay/1");
  client.subscribe("/myroom/relay/2");
  client.subscribe("/myroom/relay/3");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    digitalWrite(2, LOW);

    // Attempt to connect
    if (client.connect("ESPRelay", mqtt_username, mqtt_password)) {
      Serial.println("Connected");
      digitalWrite(2, HIGH);
      // Once connected, publish an announcement...

      //publish on connect insert here
      client.publish("/myroom/relay/boot", "0"); //publish to topic on boot
      
      sprintf(ipaddr, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );
      
      client.publish("/myroom/relay/ip", ipaddr);
      // ... and resubscribe
      subscribemqtt();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      
      //Serial.println(" try again in 100 miliseconds");
      // Wait before retrying
      //delay(100);
    }
  }
}
