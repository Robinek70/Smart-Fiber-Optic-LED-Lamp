#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <PubSubClient.h>
#include <WebSocketsServer.h>
#include <LittleFS.h>
#include "HtmlColor.h"
#include <FastLED.h>
#include <PinButton.h>
//#include <EEPROM.h>

// define the LEDs
#define LED_PIN  2 		// D4 //pin the LEDs are connected to
#define NUM_LEDS 32
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
struct CRGB leds[NUM_LEDS];

DNSServer dnsServer;
ESP8266WebServer server (80);
IPAddress APIP (192, 168, 4, 1);
WiFiClient client;
PubSubClient mqttClient(client);

byte isOn = 1;
byte requestedState = 1;
int setMode = 2;
uint8_t maxBrightness = 160;
uint16 updatesPerSec = 200;
bool isAutoChange = false;
uint32_t nextChangeTick = 0;
bool nextChange = false;

#include "solid_color_mode.h"
#include "palette_mode.h"
#include "effect_mode.h"
#include "settings.h"
#include "wifi_logo.h"
#include "wifi.h"
#include "web.h"
#include "mqtt.h"

const int wakeUpPin = 0; // D3
const int gndPin = 4;    // D2
PinButton FunctionButton(wakeUpPin);

void setup() {
	Serial.begin(115200);
	while (!Serial);

	pinMode(gndPin, OUTPUT);
	digitalWrite(gndPin, LOW);

	FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
	FastLED.setBrightness(maxBrightness);
	FastLED.clear();
	Serial.println("\nFastled started");

	load_settings();

	leds[0].setRGB(255, 0, 0);
	leds[8].setRGB(0, 255, 0);
	leds[20].setRGB(0, 0, 255);
	FastLED.show();
	delay(800);
	
	setup_wifi();
	setupMqtt();
	nextChangeTick = 0;
}



void loop() {
	FunctionButton.update();

	if(FunctionButton.isSingleClick()) {
		nextChange = true;
	}

	if (nextChange) {	// fade colors
		if (setMode == 0) {
			colorCounter++;
			if (colorCounter > 17) {
				colorCounter = 0;
			}
		}
		else if (setMode == 1) {
			paletteCounter++;
			if (paletteCounter > 11) { // adjust if you have more or less than 34 palettes
				paletteCounter = 0;
			}
		}
		else if (setMode == 2) {
			nextPattern();  // Change to the next pattern
		}
	}
	else if (FunctionButton.isDoubleClick()) {
        Serial.println("isDoubleClick, Mode: ");
		setMode++;
		if (setMode > 2) {
			setMode = 0;
		}
		Serial.println(setMode);
	}
	else if (FunctionButton.isLongClick()) {
    	Serial.print("isLongClick: ");

		requestedState = !requestedState;
	}

    nextChange = false;
	if(isAutoChange && (nextChangeTick < millis())) {
		nextChangeTick = millis() + 1000*settings.at (F("changeInterval")).toInt();
		nextChange=true;
	}

	if (!mqttClient.connected()) {
		reconnect();
	} else {
		mqttClient.loop();
	}

	server.handleClient ();

    if(requestedState != isOn) {
		isOn = requestedState;
		mqttClient.publish(mqttTopicState, ((isOn) ? STATE_ON : STATE_OFF) , true);
		if(!isOn) {
			save_settings();
		}
		Serial.println(isOn?"Turn ON":"Turn OFF");
	}

	if(!isOn) {
		fadeToBlackBy( leds, NUM_LEDS, 20);
		FastLED.show();
		return;
	}

	if (setMode == 0) {
		if (colorCounter % 2 == 0) {
		float breath = (exp(sin(millis()/2000.0*PI)) - 0.36787944)*108.0;
		FastLED.setBrightness(breath);
		}
		else {
			FastLED.setBrightness(maxBrightness);
		}
		ChangeColorPeriodically();
	}
	else if (setMode == 1) {
		FastLED.setBrightness(maxBrightness);
		ChangePalettePeriodically();
		static uint8_t startIndex = 0;
		startIndex = startIndex + 1;
		FillLEDsFromPaletteColors(startIndex);
	}
	else if (setMode == 2) {
		gPatterns[gCurrentPatternNumber]();
	}

	FastLED.show();
	FastLED.delay(4000 / updatesPerSec);
	EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
}
