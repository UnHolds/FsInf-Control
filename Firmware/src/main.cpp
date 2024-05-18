#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS 6
CRGB leds[NUM_LEDS];
#define WS2812_PIN 22

void setup() {
    for(int i = 0; i < NUM_LEDS; i++){
        leds[i] = CRGB(0,  0,  50);//CRGB::Black;
    }
    FastLED.addLeds<WS2812B, WS2812_PIN, RGB>(leds, NUM_LEDS);

    FastLED.show();
}

void loop() {

}
