#pragma once

void wifi_logo(CRGB color) {
    static int index = 0;
    FastLED.clear();
    leds[++index%NUM_LEDS] = color;
    FastLED.show();
}