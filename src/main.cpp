#include <WS2812Serial.h>
#define USE_WS2812SERIAL
#include <Arduino.h>
#include <FastLED.h>
#include <stdint.h>
// #define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>

#define KNOB_CLK 3
#define KNOB_DT 2
#define KNOB_SW 1
#define KNOB_PLUS 0

#define LED_PIN_A 8
#define LED_PIN_B 17
#define LED_PIN_C 14
#define LED_PIN_D 20

#define NUM_LEDS 144
#define NUM_STRIPS 4

#define COLOR_ORDER BRG

#define LED_MODE_PILLARS 0
#define LED_MODE_DOUBLE_RAINBOW 1
#define LED_MODE_SPARKLE 2
#define LED_MODE_RAINBOW 3
#define MAX_LED_MODE 3

#define MIN_BRIGHTNESS 5
#define MAX_BGRIGHTNESS 50

#define MODE_OFF 0
#define MODE_LED 1
#define MODE_BRIGHTNESS 2

CRGB leds[NUM_STRIPS][NUM_LEDS];

int32_t mode = MODE_LED;

int32_t brightness = 10;
byte hue = 0;
int32_t led_mode = LED_MODE_DOUBLE_RAINBOW;
byte led_pos = 0;
int abs_led_pos = 0;

Encoder my_enc(KNOB_DT, KNOB_CLK);

int32_t enc_led_mode_pos = 0;
int32_t enc_brightness_pos = MIN_BRIGHTNESS * 10;

unsigned long last_button_press = 0;
byte fast_button_count = 3;

void led_standby()
{
    FastLED.setBrightness(MIN_BRIGHTNESS);

    for (byte s = 0; s < NUM_STRIPS; s++)
    {
        leds[s][0] = CHSV(0, 255, 255);
        for (byte i = 1; i < NUM_LEDS; i++)
        {
            leds[s][i] = CRGB::Black;
        }
    }

    FastLED.show();
}

void setup_lights()
{
    FastLED.addLeds<WS2812SERIAL, LED_PIN_A, COLOR_ORDER>(leds[0], NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.addLeds<WS2812SERIAL, LED_PIN_B, COLOR_ORDER>(leds[1], NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.addLeds<WS2812SERIAL, LED_PIN_C, COLOR_ORDER>(leds[2], NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.addLeds<WS2812SERIAL, LED_PIN_D, COLOR_ORDER>(leds[3], NUM_LEDS).setCorrection(TypicalLEDStrip);

    delay(100);

    led_standby();
}

void set_led_mode()
{
    mode = MODE_LED;
    my_enc.write(enc_led_mode_pos);
}

void set_brightness_mode()
{
    mode = MODE_BRIGHTNESS;
    my_enc.write(enc_brightness_pos);
}

void sw()
{

    long button_delta = millis() - last_button_press;
    // if 50ms have passed since last LOW pulse, it means that the
    // button has been pressed, released and pressed again
    if (button_delta > 50)
    {
        if (button_delta > 1000)
        {
            fast_button_count = 0;
            if (mode == MODE_OFF)
            {
                set_led_mode();
            }
            else if (mode == MODE_LED)
            {
                set_brightness_mode();
            }
            else if (mode == MODE_BRIGHTNESS)
            {
                set_led_mode();
            }

            if (mode == MODE_OFF)
            {
                led_standby();
            }
        }
        else
        {
            fast_button_count++;
            if (fast_button_count >= 3)
            {
                mode = MODE_OFF;
                led_standby();
                fast_button_count = 0;
            }
        }
    }

    // Remember last button press event
    last_button_press = millis();
}

void setup_knob()
{
    pinMode(KNOB_PLUS, OUTPUT);

    delay(100);

    digitalWrite(KNOB_PLUS, HIGH);
    delay(100);

    pinMode(KNOB_SW, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(KNOB_SW), sw, LOW);
}

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    delay(10);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(3000);
    digitalWrite(LED_BUILTIN, LOW);

    setup_lights();
    setup_knob();
}

void read_knob()
{
    int32_t new_pos = my_enc.read();

    if (mode == MODE_BRIGHTNESS)
    {
        if (new_pos > MAX_BGRIGHTNESS * 10)
        {
            new_pos = MAX_BGRIGHTNESS * 10;
        }
        else if (new_pos < MIN_BRIGHTNESS * 10)
        {
            new_pos = MIN_BRIGHTNESS * 10;
        }
    }
    else if (mode == MODE_LED)
    {
        if (new_pos > MAX_LED_MODE * 10)
        {
            new_pos = MAX_LED_MODE * 10;
        }
        else if (new_pos < MODE_OFF * 10)
        {
            new_pos = MODE_OFF * 10;
        }
    }

    my_enc.write(new_pos);

    if (mode == MODE_BRIGHTNESS)
    {
        if (round(new_pos / 10) != brightness)
        {
            brightness = round(new_pos / 10);
            if (brightness < MIN_BRIGHTNESS)
            {
                FastLED.setBrightness(MIN_BRIGHTNESS);
            }
            else
            {
                FastLED.setBrightness(brightness);
            }
        }
        enc_brightness_pos = new_pos;
    }
    else if (mode == MODE_LED)
    {
        led_mode = round(new_pos / 10);
        enc_led_mode_pos = new_pos;
    }
}

void inc_pos()
{
    led_pos++;
    if (led_pos >= NUM_LEDS)
    {
        led_pos = 0;
    }
}

void inc_abs_pos()
{
    abs_led_pos++;
    if (abs_led_pos >= NUM_LEDS * 4)
    {
        abs_led_pos = 0;
    }
}

byte red = 0;
byte yellow = 64;
byte green = 96;
byte blue = 160;
byte pilars[NUM_STRIPS] = {red, yellow, green, blue};

byte active_strip = 0;
int slow_hue = 0;

void inc_strip()
{
    active_strip++;
    if (active_strip >= NUM_STRIPS)
    {
        active_strip = 0;
    }
}

void inc_slow_hue()
{
    slow_hue++;
    if (slow_hue >= 256 * 4)
    {
        slow_hue = 0;
    }
}

void led_pillars()
{
    for (byte s = 0; s < NUM_STRIPS; s++)
    {
        fade_raw(leds[s], NUM_LEDS, 5);
    }

    leds[active_strip][led_pos].setHue(slow_hue / 4);
    inc_strip();
    leds[active_strip][led_pos].setHue(slow_hue / 4);
    inc_strip();
    leds[active_strip][led_pos].setHue(slow_hue / 4);

    inc_slow_hue();
    inc_pos();
}

void led_double_rainbow()
{
    for (byte s = 0; s < NUM_STRIPS; s++)
    {
        fade_raw(leds[s], NUM_LEDS, 1);
    }
    byte led_pos_rev = NUM_LEDS - led_pos - 1;
    byte hue_rev = (slow_hue / 4) + 128;

    for (byte s = 0; s < NUM_STRIPS; s++)
    {
        if (s % 1 == 0)
        {
            leds[s][led_pos].setHue(slow_hue / 4);
            leds[s][led_pos_rev].setHue(hue_rev);
        }
        else
        {
            leds[s][led_pos_rev].setHue(slow_hue / 4);
            leds[s][led_pos].setHue(hue_rev);
        }
    }

    inc_slow_hue();

    inc_pos();
    inc_pos();
}

void led_sparkle()
{
    byte pos = 0;
    byte s = 0;
    if (rand() % 100 > 90)
    {
        s = rand() % NUM_STRIPS;
        pos = rand() % NUM_LEDS;
        leds[s][pos] = CRGB::White;
    }
    s = rand() % NUM_STRIPS;
    pos = rand() % NUM_LEDS;
    leds[s][pos] = CRGB::Black;
    s = rand() % NUM_STRIPS;
    pos = rand() % NUM_LEDS;
    leds[s][pos] = CRGB::Black;
}

void led_rainbow()
{
    for (byte s = 0; s < NUM_STRIPS; s++)
    {
        fade_raw(leds[s], NUM_LEDS, 1);
    }
    leds[abs_led_pos / NUM_LEDS][abs_led_pos % NUM_LEDS].setHue(slow_hue / 4);

    inc_slow_hue();
    inc_abs_pos();
}

void loop_leds()
{
    if (led_mode == LED_MODE_PILLARS)
    {
        led_pillars();
    }
    else if (led_mode == LED_MODE_DOUBLE_RAINBOW)
    {
        led_double_rainbow();
    }
    else if (led_mode == LED_MODE_SPARKLE)
    {
        led_sparkle();
    }
    else if (led_mode == LED_MODE_RAINBOW)
    {
        led_rainbow();
    }
    FastLED.show();
}

void loop()
{
    if (mode == MODE_OFF)
    {
        delay(1000);
    }
    else
    {
        read_knob();
        loop_leds();
        delay(5);
    }
}
