#pragma once

#include <stdint.h>

#define STATE_ON    "ON"
#define STATE_OFF   "OFF"

uint32_t mqttConnectionTime = 0;
String mqttClientName;
char mqttTopicState[64];
char mqttTopicStatus[64];
char mqttTopicSet[64];
char mqttTopicIp[64];

void setMqttClientName() {
  mqttClientName = ("FiberLamp_"+String(ESP.getChipId(), HEX));
}

void mqttCallback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  String t = String(topic);
  if (t.startsWith(mqttTopicSet))
  {
    int pos = t.lastIndexOf("/");
    String toSet = t.substring(pos + 1);

    for (auto &key_value : settings)
    {
      auto &key = key_value.first;
      auto &value = key_value.second;
      if (key == toSet)
      {
        value = messageTemp;
        Serial.print(key);
        Serial.print("->");
        Serial.println(value);
      }
    }

    update();
  }
}

bool reconnect() {
  if(settings.at (F("mqttHost")).length()==0) {
    return false;
  }
  if(mqttConnectionTime+5000 > millis()) {
    return mqttClient.connected();
  }
  
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    Serial.print("MQTT client name: ");
    Serial.println(mqttClientName);
    mqttConnectionTime = millis();
    if (mqttClient.connect(mqttClientName.c_str(), NULL, NULL, mqttTopicStatus, 1, true, "offline")) {
      Serial.println("connected"); 
      mqttConnectionTime = 0;

      mqttClient.publish(mqttTopicStatus, "online", true);
      mqttClient.publish(mqttTopicState, ((isOn) ? STATE_ON : STATE_OFF) , true);

      char buf[20];
      sprintf(buf, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
      mqttClient.publish(mqttTopicIp, buf, true);

      mqttClient.subscribe((String(mqttTopicSet)+'#').c_str());

      Serial.print("subscribed to ");
      Serial.println(mqttTopicSet);

    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
    }

  return mqttClient.connected();
}

void setupMqtt()
{
  auto mqtt_server = settings.at (F("mqttHost")).c_str();
  Serial.print(F("MQTT server: "));
  Serial.print (mqtt_server);
  Serial.println (F(":1883"));
 
  setMqttClientName();
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(mqttCallback);

  auto mqttTopicPrefix = settings.at (F("mqttTopic")).c_str();

  sprintf(mqttTopicState, "%sstate", mqttTopicPrefix);
  sprintf(mqttTopicStatus, "%sstatus", mqttTopicPrefix);
  sprintf(mqttTopicSet, "%sset/", mqttTopicPrefix);
  sprintf(mqttTopicIp, "%sip", mqttTopicPrefix);
}