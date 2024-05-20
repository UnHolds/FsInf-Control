#include <Arduino.h>
#include <FastLED.h>
#include <SPI.h>

#include "AudioData.h"

#define NUM_LEDS 6
CRGB leds[NUM_LEDS];
#define WS2812_PIN 22

#define CS_PIN 5
int audio_idx = 3;
SPIClass *vspi = new SPIClass(VSPI);

#define MASK 0b0011111111110000
#define GAIN_1 0x2000
#define ACTIVE 0x1000

#define BTN_1 21
#define BTN_2 25
#define BTN_3 26
#define BTN_4 27
#define AMP_SHDN 33

#define BOUNCE_TIME 5

//state vars
bool is_fsinf_open = false;
int is_fsinf_open_buffer = 0;
#define IS_FSINF_OPEN_BUFFER_CAP 1000

uint8_t dim_value = 128;

bool playSound = false;
bool playLed = false;

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

void set_top_leds(CRGB color){
    leds[2] = color;
    leds[3] = color;
    leds[4] = color;
    leds[5] = color;
    led_change = true;
}

void setup() {
    vspi->begin();
    pinMode(CS_PIN, OUTPUT);


    pinMode(BTN_1, INPUT);
    pinMode(BTN_2, INPUT);
    pinMode(BTN_3, INPUT);
    pinMode(BTN_4, INPUT);

    pinMode(AMP_SHDN, OUTPUT);
    digitalWrite(AMP_SHDN, LOW);

    Serial.begin(115200);

    //setup leds
    leds_off();
    FastLED.addLeds<WS2812B, WS2812_PIN, GRB>(leds, NUM_LEDS);

    set_top_leds(CRGB::Red);
    is_fsinf_open = false;

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
        leds[0] = CRGB::Black;
        leds[1] = CRGB::Black;
        led_change = true;
        digitalWrite(AMP_SHDN, LOW);
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

    if(digitalRead(BTN_1) == HIGH && btn1_bounce == 0 && btn1_pressed == false){
        btn1_bounce = BOUNCE_TIME;
        btn1_pressed = true;
        btn1_changed = true;
    }else if(digitalRead(BTN_1) == LOW && btn1_bounce == 0 && btn1_pressed == true){
        btn1_bounce = BOUNCE_TIME;
        btn1_pressed = false;
        btn1_changed = true;
    }

    if(digitalRead(BTN_2) == HIGH && btn2_bounce == 0 && btn2_pressed == false){
        btn2_bounce = BOUNCE_TIME;
        btn2_pressed = true;
        btn2_changed = true;
    }else if(digitalRead(BTN_2) == LOW && btn2_bounce == 0 && btn2_pressed == true){
        btn2_bounce = BOUNCE_TIME;
        btn2_pressed = false;
        btn2_changed = true;
    }

    if(digitalRead(BTN_3) == HIGH && btn3_bounce == 0 && btn3_pressed == false){
        btn3_bounce = BOUNCE_TIME;
        btn3_pressed = true;
        btn3_changed = true;
    }else if(digitalRead(BTN_3) == LOW && btn3_bounce == 0 && btn3_pressed == true){
        btn3_bounce = BOUNCE_TIME;
        btn3_pressed = false;
        btn3_changed = true;
    }

    if(digitalRead(BTN_4) == HIGH && btn4_bounce == 0 && btn4_pressed == false){
        btn4_bounce = BOUNCE_TIME;
        btn4_pressed = true;
        btn4_changed = true;
    }else if(digitalRead(BTN_4) == LOW && btn4_bounce == 0 && btn4_pressed == true){
        btn4_bounce = BOUNCE_TIME;
        btn4_pressed = false;
        btn4_changed = true;
    }
}



void handleEvents() {
    if(btn1_pressed && btn1_changed){
        btn1_changed = false;
        playSound = true;
        leds[0] = CRGB::Cyan;
        leds[1] = CRGB::Cyan;
        led_change = true;
        digitalWrite(AMP_SHDN, LOW);

    }
    if(btn2_pressed && btn2_changed){
        btn2_changed = false;
        if(is_fsinf_open) {
            set_top_leds(CRGB::Red);
            is_fsinf_open = false;
        }else{
            set_top_leds(CRGB::Green);
            is_fsinf_open = true;
        }
    }
    if(btn3_pressed && btn3_changed){
        btn3_changed = false;
        dim_value = (dim_value + 1) % 5;
        FastLED.setBrightness(32 + (50 * dim_value));
        led_change = true;
    }
    if(btn4_pressed && btn4_changed){
        btn3_changed = false;
        playLed = true;
    }
}

void handle_led(){
    if(led_change){
        led_change = false;
        FastLED.show();
    }
}

int16_t r_val = 0;
int16_t g_val = 0;
int16_t b_val = 0;
int16_t led_mode = 0;
uint8_t led_inc = 5;
void play_led() {
    if(playLed == false){
        return;
    }

    if(led_mode == 6){
        led_mode = 0;
        leds_off();
        if(is_fsinf_open){
            set_top_leds(CRGB::Green);
        } else {
            set_top_leds(CRGB::Red);
        }
        playLed = false;
        return;
    }

    if(r_val >= 255 && g_val <= 0 && b_val <= 0){
        led_mode += 1;
    }

    if(r_val >= 255 && g_val < 255 && b_val <= 0){
        g_val += led_inc;
    }else if(r_val > 0 && g_val >= 255 && b_val <= 0){
        r_val -= led_inc;
    }else if(r_val <= 0 && g_val >= 255 && b_val < 255){
        b_val += led_inc;
    }else if(r_val <= 0 && g_val > 0 && b_val >= 255){
        g_val -= led_inc;
    }else if(r_val < 255 && g_val <= 0 && b_val >= 255){
        r_val += led_inc;
    }else if(r_val >= 255 && g_val <= 0 && b_val > 0){
        b_val -= led_inc;
    }else{
        r_val = 255;
        g_val = 0;
        b_val = 0;
    }

    set_top_leds(CRGB(r_val, g_val, b_val));
    leds[0] = CRGB(r_val, g_val, b_val);
    leds[1] = CRGB(r_val, g_val, b_val);

}


void handle_serial(){
    if(Serial.available() == 0){
        return;
    }

    switch(Serial.read()){
        case 'O':
        case 'o':
            is_fsinf_open = true;
            set_top_leds(CRGB::Green);
        break;
        case 'C':
        case 'c':
            is_fsinf_open = true;
            set_top_leds(CRGB::Red);
        break;
        case '1':
            playSound = true;
            digitalWrite(AMP_SHDN, LOW);
        break;

    }
}

void loop()
{
    if(next_check < micros()){
        handleButtons();
        handleEvents();
        play_led();
        handle_led();
        handle_serial();
        next_check = micros() + 10000;

        if(is_fsinf_open_buffer == IS_FSINF_OPEN_BUFFER_CAP){
            is_fsinf_open_buffer = 0;
            Serial.print('O');
        }else if(is_fsinf_open_buffer == -IS_FSINF_OPEN_BUFFER_CAP){
            is_fsinf_open_buffer = 0;
            Serial.print('C');
        }else{
            is_fsinf_open_buffer += is_fsinf_open ? 1 : -1;
        }
    }

    long currentTime = micros();
    if (playSound && (currentTime - timing) >= (1000000 / sample_rate))
    {
        timing = currentTime;
        sendAudioData();
    }
}
