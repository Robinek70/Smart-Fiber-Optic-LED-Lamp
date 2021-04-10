#pragma once

#include "web.h"
#include "settings.h"

void setup_wifi() {
  Serial.println("Wifi starting...");
  WiFi.mode (WIFI_STA);
  WiFi.persistent (false);
  if (settings.at (F("ssid")) != "")
  {
    Serial.print (F("\nConnecting to "));
    Serial.print (settings.at (F("ssid")));
    WiFi.begin (settings.at (F("ssid")), settings.at (F("password")));
  

    // Wait for WiFi to connect
    auto timeout = millis () + (30 * 1000);
    while (WiFi.status () != WL_CONNECTED && millis () < timeout)
    {
      wifi_logo (CRGB( 100, 50, 50));
      delay (15);
    }
  }

  if (WiFi.status () == WL_CONNECTED)
  {
    
    Serial.print (F("\nIP address: "));
    Serial.println (WiFi.localIP ());

    MDNS.begin (F("smartfiberlamp"));

    // show swirl ;)
    for (int N = 0x200; N >= 0; N -= 3)
    {
      wifi_logo (CRGB (N > 0x180 ? N - 0x180 : 0, N > 0x100 ? 0x80 : (N > 0x80 ? N - 0x80 : 0), N > 0x180 ? N - 0x180 : 0));
      delay (15);
    }
  }
  else
  {      
    // Start SoftAP mode
    Serial.print (F("\nAP mode: "));
    Serial.println (APIP);
    WiFi.mode (WIFI_AP);
    WiFi.softAPConfig (APIP, APIP, IPAddress (255, 255, 255, 0));
    WiFi.softAP (F("FiberLamp"));
    dnsServer.start (53, "*", APIP);
    for (int N = 0x200; N >= 0; N -= 3)
    {
      wifi_logo (CRGB (N > 0x180 ? N - 0x180 : 0, N > 0x180 ? N - 0x180 : 0, N > 0x100 ? 0x80 : (N > 0x80 ? N - 0x80 : 0)));
      delay (55);
    }
  }
  
  //Apply settings
  update ();

  // Start the webserver
  server.on (F("/"), handle_root);
  server.on (F("/store"), handle_update);
  server.on (F("/change"), handle_change);
  server.on (F("/reboot"), handle_reboot);
  server.on (F("/pure-min.css"), [] { server.send (200, "text/css", pure_min_css); });
  server.on (F("/grids-responsive-min.css"), [] { server.send (200, "text/css", grids_responsive_min_css); });
  server.on (F("/script.js"), [] { server.send (200, "text/js", script_js); });
  server.onNotFound (handle_root);
  server.begin ();

  Serial.println(F("Webserver started."));
}