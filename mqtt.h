#pragma once

#include <stdint.h>

uint32_t mqttConnectionTime = 0;
String mqttClientName;
char mqttTopicState[64];
char mqttTopicStatus[64];
char mqttTopicSet[64];
char mqttTopicIp[64];

void setMqttClientName() {
  mqttClientName = ("FiberLamp_"+String(ESP.getChipId(), HEX));
}

void publishState(const __FlashStringHelper* property) {
    if(!mqttClient.connected())
      return;
  
  char topic[40];
  sprintf(topic, "%s/%S", mqttTopicState, property);
  mqttClient.publish(topic, settings.at (property).c_str(), false);
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

  if(t == mqttTopicSet+String("fx")) {
    int pos = messageTemp.indexOf("|");
    auto mode =  messageTemp.substring(0,pos);
    auto effect = messageTemp.substring(pos+1);
    settings["mode"] = mode;

    if(mode == "Effect") settings["effectNumber"] = effect;
    if(mode == "Solid") settings["colorCounter"] = effect;
    if(mode == "Palette") settings["paletteCounter"] = effect;

    Serial.println(mode);
    Serial.println(effect);

    update();
    return;
  }

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

const char haConfig[] PROGMEM = R""(
{
  "name": "%s", "unique_id": "%s", "cmd_t": "%sstate", "stat_t": "%s", "avty_t": "%s"
, "bri_scl": 255, "platform": "mqtt"
, "device": {"name": "%s", "ids": ["%s"], "model": "Fiber Optic Lamp", "mf": "Robinek70", "cns": [["IP", "%s"],["MAC", "%s"]]}
, "bri_cmd_t": "%smaxBrightness", "bri_stat_t": "%s/maxBrightness"
, "effect_list": [%s]
, "fx_cmd_t": "%sfx"
})"";

const char haConfigAc[] PROGMEM = R""(
{
  "name": "%s_ac", "unique_id": "%s_ac", "cmd_t": "%sautochange", "stat_t": "%s/autochange", "avty_t": "%s"
, "platform": "mqtt"
, "device": {"name": "%s", "ids": ["%s"], "model": "Fiber Optic Lamp", "mf": "Robinek70", "cns": [["IP", "%s"],["MAC", "%s"]]}
})"";

//, "icon": "mdi:lightbulb-cfl"
//, "icon": "mdi:repeat"

template<typename List>
String make_list (const List &list, const char * prefix)
{
  String choices;
  for (const auto &item : list)
  {
    choices += '"';
    choices += prefix;
    choices += '|';
    choices += item;
    choices += '"';
    choices += ',';
  }
  return choices;
}


void publishToHa() {
    if(settings.at (F("haDiscovery")) == STATE_ON) {
      char mqttTopicHa[64];

      String effects;
      effects += make_list(color_names, "Solid");
      effects += make_list(palette_names, "Palette");
      effects += make_list(effect_names, "Effect");
      effects += '"';
      effects += '"';
      char payload[1500];

      Serial.println (F("HA config"));

      mqttClient.setBufferSize(1500);

      sprintf_P(payload, haConfig, 
        mqttClientName.c_str(), mqttClientName.c_str(), mqttTopicSet, mqttTopicState, mqttTopicStatus // name, id, cmd, stat, avty
        , mqttClientName.c_str(), String(ESP.getChipId(), HEX).c_str(), WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str()  // dev name, ids, IP, MAC
        , mqttTopicSet, mqttTopicState  // bri_cms, bri_stat
        , effects.c_str()
        , mqttTopicSet);

      sprintf_P(mqttTopicHa, PSTR("homeassistant/light/%s/config"), mqttClientName.c_str());
      Serial.println (mqttTopicHa);
      Serial.println (payload);
      Serial.println (strlen(payload));
      mqttClient.publish(mqttTopicHa, payload, true);

      sprintf_P(payload, haConfigAc, 
        mqttClientName.c_str(), mqttClientName.c_str(), mqttTopicSet, mqttTopicState, mqttTopicStatus
        , mqttClientName.c_str(), String(ESP.getChipId(), HEX).c_str(), WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str()
      );

      sprintf_P(mqttTopicHa, PSTR("homeassistant/light/%s/ac/config"), mqttClientName.c_str());
      Serial.println (mqttTopicHa);
      Serial.println (payload);
      Serial.println (strlen(payload));
      mqttClient.publish(mqttTopicHa, payload, true);

      mqttClient.setBufferSize(256); 
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

      publishState(F("maxBrightness"));
      publishState(F("autochange"));

      char buf[20];
      sprintf(buf, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);

      mqttClient.publish(mqttTopicIp, buf, true);

      mqttClient.subscribe((String(mqttTopicSet)+'#').c_str());

      Serial.print("subscribed to ");
      Serial.println(mqttTopicSet);

      publishToHa();

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
