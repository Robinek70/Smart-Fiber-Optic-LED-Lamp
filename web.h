#pragma once

#include <list>
#include "page_template.h"

template<typename List>
String make_options (const List &list, const String &current)
{
  String choices;
  for (const auto &item : list)
  {
    choices += F(R""(<option value=")"");
    choices += item;
    choices += '"';
    if (String (FPSTR(item)) == current)
    {
      choices += F( " selected");
    }
    choices += ">";
    choices += item;
    choices += F(R""(</option>)"");
  }
  return choices;
}

void replaceVariables(String& result) {
  for (const auto &setting : settings)
  { 
    result.replace ("{{" + setting.first + "}}", setting.second);
  }

  // replace placeholders
  result.replace (F("{{modes_choices}}"), make_options (modes_names, settings.at (F("mode"))));
  result.replace (F("{{color_choices}}"), make_options (color_names, settings.at (F("colorCounter"))));
  result.replace (F("{{palette_choices}}"), make_options (palette_names, settings.at (F("paletteCounter"))));
  result.replace (F("{{effect_choices}}"), make_options (effect_names, settings.at (F("effectNumber"))));
  result.replace (F("{{hide_net}}"), settings.at (F("ssid"))==""?"":"hide");
  result.replace (F("{{stateCheck}}"), isOn?"checked":"");
}

void handle_root ()
{
  String result;
  result.reserve (13000); 
  result += FPSTR (page_template);

  replaceVariables(result);
 
  // Send the completed page
  server.sendHeader ("cache-control", "max-age=10");
  server.send (200, F("text/html"), result);
}

void updateFromServer() {
  for (auto &key_value : settings)
  {
    auto &key = key_value.first;
    auto &value = key_value.second;
    if (server.hasArg (key))
    {
      value = server.arg (key);
      Serial.print(key);
      Serial.print("->");
      Serial.println(value);
    }
  }
  update ();
}


void handle_update () 
{
  updateFromServer();
  
  server.send (200);
}

/**************************************************************************/

// update from server and save settings
void handle_change () 
{
  updateFromServer();
  server.send (200);
  save_settings ();
}

/*
 * Reboot the lamp
 */
const char reboot_template[] PROGMEM = 
R""(
<!DOCTYPE html>
<html>
   <body>
      <script>
         setTimeout(function(){
            window.location.href = 'http://{{IPLocal}}';
         }, 10000);
      </script>
      <p>Rebooting...</p>
   </body>
</html>)"";
 
void handle_reboot ()
{
  String result;
  result.reserve (3000); 
  result += FPSTR (reboot_template);
  result.replace (F("{{IPLocal}}"), WiFi.localIP ().toString());
  server.send (200, F("text/html"), result);
  
  delay (500);
  ESP.restart ();
}
