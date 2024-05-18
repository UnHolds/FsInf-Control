#include <Arduino.h>
#include <FastLED.h>
#include <SPI.h>

#include "AudioData.h"

#define NUM_LEDS 6
CRGB leds[NUM_LEDS];
#define WS2812_PIN 22

#define CS_PIN 5
int audio_idx = 0;
SPIClass *vspi = new SPIClass(VSPI);

#define MASK 0b0011111111110000
#define GAIN_1 0x2000
#define ACTIVE 0x1000

#define BTN_1 21
#define BTN_2 25
#define BTN_3 26
#define BTN_4 27

#define BOUNCE_TIME 5

bool playSound = false;
bool btn1_pressed = false;
bool btn1_changed = true;
bool btn2_pressed = false;
bool btn2_changed = true;
bool btn3_pressed = false;
bool btn3_changed = true;
bool btn4_pressed = false;
bool btn4_changed = true;
uint8_t btn1_bounce = 0;
uint8_t btn2_bounce = 0;
int8_t btn3_bounce = 0;
uint8_t btn4_bounce = 0;
unsigned long next_check = 0;

long timing = micros();

bool led_change;

void leds_off(){
    for(int i = 0; i < NUM_LEDS; i++){
        leds[i] = CRGB::Black;
    }
}

void setup() {
    vspi->begin();
    pinMode(CS_PIN, OUTPUT);


    pinMode(BTN_1, INPUT);
    pinMode(BTN_2, INPUT);
    pinMode(BTN_3, INPUT);
    pinMode(BTN_4, INPUT);

    //setup leds
    leds_off();
    FastLED.addLeds<WS2812B, WS2812_PIN, RGB>(leds, NUM_LEDS);

    FastLED.show();
}

void sendAudioData(){
    vspi->beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
    digitalWrite(vspi->pinSS(), LOW);
    uint16_t tdata = data[audio_idx] << 4;
    tdata |= ACTIVE;
    tdata &= MASK;
    vspi->transfer16(tdata);
    digitalWrite(vspi->pinSS(), HIGH);
    vspi->endTransaction();
    audio_idx = (audio_idx + 1) % length;
    if(audio_idx == 0){
        playSound = false;
    }
}

void handleButtons() {

    if(btn1_bounce > 0){
        btn1_bounce -= 1;
    }

    if(btn2_bounce > 0){
        btn2_bounce -= 1;
    }

    if(btn3_bounce > 0){
        btn3_bounce -= 1;
    }

    if(btn4_bounce > 0){
        btn4_bounce -= 1;
    }

    if(digitalRead(BTN_1) == HIGH && btn1_bounce == 0){
        btn1_bounce = BOUNCE_TIME;
        btn1_pressed = true;
        btn1_changed = true;
    }else if(digitalRead(BTN_1) == LOW && btn1_bounce == 0){
        btn1_bounce = BOUNCE_TIME;
        btn1_pressed = false;
        btn1_changed = true;
    }

    if(digitalRead(BTN_2) == HIGH && btn2_bounce == 0){
        btn2_bounce = BOUNCE_TIME;
        btn2_pressed = true;
        btn2_changed = true;
    }else if(digitalRead(BTN_2) == LOW && btn2_bounce == 0){
        btn2_bounce = BOUNCE_TIME;
        btn2_pressed = false;
        btn2_changed = true;
    }

    if(digitalRead(BTN_3) == HIGH && btn3_bounce == 0){
        btn3_bounce = BOUNCE_TIME;
        btn3_pressed = true;
        btn3_changed = true;
    }else if(digitalRead(BTN_2) == LOW && btn3_bounce == 0){
        btn3_bounce = BOUNCE_TIME;
        btn3_pressed = false;
        btn3_changed = true;
    }

    if(digitalRead(BTN_4) == HIGH && btn4_bounce == 0){
        btn4_bounce = BOUNCE_TIME;
        btn4_pressed = true;
        btn4_changed = true;
    }else if(digitalRead(BTN_4) == LOW && btn4_bounce == 0){
        btn4_bounce = BOUNCE_TIME;
        btn4_pressed = false;
        btn4_changed = true;
    }
}



void handleEvents() {
    if(btn1_pressed && btn1_changed){
        btn1_changed = false;
        playSound = true;

    }
    if(btn2_pressed && btn2_changed){
        btn2_changed = false;
        leds_off();
        leds[0] = CRGB(0,20,20);
        leds[1] = CRGB(0,20,20);
        led_change = true;
    }
    if(btn3_pressed && btn3_changed){
        btn3_changed = false;
        leds_off();
        led_change = true;
    }
    if(btn4_pressed && btn4_changed){
        btn3_changed = false;
        leds_off();
        leds[2] = CRGB(0,20,20);
        leds[3] = CRGB(0,20,20);
        leds[4] = CRGB(0,20,20);
        leds[5] = CRGB(0,20,20);
        led_change = true;
    }
}

void handle_led(){
    if(led_change){
        led_change = false;
        FastLED.show();
    }
}

void loop()
{
    if(next_check < micros()){
        handleButtons();
        handleEvents();
        handle_led();
        next_check = micros() + 100000;
    }

    long currentTime = micros();
    if (playSound && (currentTime - timing) >= (1000000 / sample_rate))
    {
        timing = currentTime;
        sendAudioData();
    }
}
