#pragma once

#include "HtmlColor.h"
#include <sstream>
#include <map>
#include <list>

using Settings = std::map<String, String>;

Settings settings;

// Default settings
const char default_settings[] PROGMEM = 
R""(maxBrightness=32
password=
ssid=
mqttHost=
state=on
mode=Effect
speed=200
solidColor=#881111
colorCounter=0
paletteCounter=0
effectNumber=0
autochange=
changeInterval=30
mqttHost=
mqttTopic=FiberLamp/
)"";

const char *modes_names[] PROGMEM =
{
  "Solid",
  "Palette",
  "Effect",
};
const char *color_names[] PROGMEM =
{
  "Red fade",
  "Red",
  "Yellow fade",
  "Yellow",
  "Yellow fade 2",
  "Yellow 2",
  "Green fade 3",
  "Green 3",
  "Blue fade",
  "Blue",
  "Dark blue fade",
  "Dark blue",
  "Purple fade",
  "Purple",
  "Pink fade",
  "Pink",
  "White fade",
  "White",
};
const char *palette_names[] PROGMEM =
{
  "rainbow_gp",
  "es_landscape_24_gp",
  "Gummy_Bears_gp",
  "bhw3_01_gp",
  "BlacK_Magenta_Red_gp",
  "BlacK_Blue_Magenta_White_gp",
  "Sunset_Real_gp",
  "lava_gp",
  "GMT_drywet_gp",
  "gr65_hult_gp",
  "departure_gp",
  "Analogous_1_gp",
};

const char *effect_names[] PROGMEM =
{
  "twoDots", "fillAndCC", "blinkyblink2", "spewFour", "spew", "confetti_GB", "rainbow", "applause", "confetti", "sinelon", "juggle"
};

template<typename List>
int findInList(const List &list, const String &current, int notFoundValue = -1) {
    int i = 0;
    for (const auto &item : list) 
    {
        if (String (FPSTR(item)) == current)
        {
          return i;
        }
        i++;
    }
    return notFoundValue;
}

void save_settings();

void update() {
  String sState = settings.at (F("state"));
  bool newState = (sState =="ON") ||(sState =="on") || (sState =="1");
  /*if(newState != isOn) {
    save_settings();
  }*/
  requestedState = newState;

  updatesPerSec = settings.at (F("speed")).toInt();
  isAutoChange = settings.at (F("autochange"))=="checked";

  String mode = settings.at (F("mode"));
  setMode = findInList(modes_names, mode, 0);

  maxBrightness = settings.at (F("maxBrightness")).toInt();
  FastLED.setBrightness(maxBrightness);

  colorCounter = findInList(color_names, settings.at (F("colorCounter")), 0);
  if(colorCounter==0) colorCounter = settings.at (F("colorCounter")).toInt();
  
  paletteCounter = findInList(palette_names, settings.at (F("paletteCounter")), 0);
  if(paletteCounter==0) paletteCounter = settings.at (F("paletteCounter")).toInt();
 
  gCurrentPatternNumber = findInList(effect_names, settings.at (F("effectNumber")), 0);
  if(gCurrentPatternNumber==0) gCurrentPatternNumber = settings.at (F("effectNumber")).toInt();

  Serial.println("Settings applied.");
}

void parse_settings (const char *data, char splitter = '\n')
{
  std::string string (data);
  std::istringstream iss (string);
  while (!iss.eof ())
  {
    std::string key, value;
    std::getline (iss, key, '=');
    std::getline (iss, value, splitter);
    if (!key.empty ())
    {
      settings[key.c_str ()] = value.c_str ();
    }
  }  
}

void load_settings ()
{
  // Load defaults
  parse_settings (String (FPSTR (default_settings)).c_str ());

  // Read from flash
  LittleFS.begin ();
  auto file = LittleFS.open (F("fiberlamp_settings"), "r");
  auto bytes = file.size ();
  auto buffer = reinterpret_cast<char *> (malloc (bytes + 1));
  file.read (reinterpret_cast<uint8_t *> (buffer), bytes);
  buffer[bytes] = '\0';
  file.close ();

  parse_settings (buffer);
  free (buffer);
}

/**************************************************************************/

void save_settings ()
{
  String str;
  for (const auto &key_value : settings)
  {
    str += key_value.first;
    str += '=';
    str += key_value.second;
    str += '\n';
  }
  
  // Save changes
  auto file = LittleFS.open (F("fiberlamp_settings"), "w");
  file.write (str.c_str (), str.length ());
  file.close ();
  Serial.println("Settings stored.");
}