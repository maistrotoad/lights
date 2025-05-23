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

#define LED_MODE_LANTERN 0
#define LED_MODE_RAINBOW_SPARKLE 1
#define LED_MODE_DOUBLE_RAINBOW 2
#define LED_MODE_SPARKLE 3
#define LED_MODE_RAINBOW 4
#define LED_MODE_PILLARS 5
#define MAX_LED_MODE 5

#define MIN_BRIGHTNESS 5
#define MAX_BRIGHTNESS 50

#define MODE_OFF 0
#define MODE_LED 1
#define MODE_BRIGHTNESS 2

CRGB leds[NUM_STRIPS][NUM_LEDS];

int32_t mode = MODE_OFF;

int32_t brightness = 10;
int32_t led_mode = LED_MODE_DOUBLE_RAINBOW;
byte led_pos = 0;
int abs_led_pos = 0;

Encoder my_enc(KNOB_DT, KNOB_CLK);

int32_t enc_led_mode_pos = 0;
int32_t enc_brightness_pos = MIN_BRIGHTNESS * 10;

unsigned long last_button_press = 0;
byte fast_button_count = 3;

bool init_led_mode = true;

void led_standby()
{
    FastLED.setBrightness(1);

    for (byte s = 0; s < NUM_STRIPS; s++)
    {
        leds[s][0] = CRGB::Red;
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
                if (led_mode == LED_MODE_LANTERN)
                {
                    FastLED.setBrightness(200);
                }
                else if (led_mode == LED_MODE_RAINBOW_SPARKLE)
                {
                    FastLED.setBrightness(20);
                }
                else
                {
                    FastLED.setBrightness(MIN_BRIGHTNESS);
                }

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
        }
        else
        {
            fast_button_count++;
            if (fast_button_count >= 3)
            {
                mode = MODE_OFF;
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
        if (new_pos > MAX_BRIGHTNESS * 10)
        {
            new_pos = MAX_BRIGHTNESS * 10;
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
        if (round(new_pos / 10) != led_mode)
        {
            if (led_mode == LED_MODE_LANTERN)
            {
                FastLED.setBrightness(MIN_BRIGHTNESS);
            }
            led_mode = round(new_pos / 10);

            if (led_mode == LED_MODE_LANTERN)
            {
                FastLED.setBrightness(200);
            }
            else if (led_mode == LED_MODE_RAINBOW_SPARKLE)
            {
                FastLED.setBrightness(20);
            }
            init_led_mode = true;
        }
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

byte active_strip = 0;
int slow_hue = 0;

void inc_strip()
{
    active_strip++;
    active_strip = active_strip % NUM_STRIPS;
}

void inc_slow_hue()
{
    slow_hue++;
    slow_hue = slow_hue % (256 * 8);
}

byte get_slow_hue()
{
    return slow_hue / 8;
}

void led_lantern()
{
    for (byte s = 0; s < NUM_STRIPS; s++)
    {
        for (byte l = 0; l < NUM_LEDS; l++)
        {
            if (l > NUM_LEDS - 10)
            {
                leds[s][l] = CRGB::White;
            }
            else
            {
                leds[s][l] = CRGB::Black;
            }
        }
    }
}

void led_pillars()
{
    byte hue = get_slow_hue();

    if (active_strip % 2 == 0)
    {
        hue = (hue + 128) % 256;
    }

    if (init_led_mode)
    {
        leds[active_strip][led_pos].setHue(hue);
        inc_pos();
        if (led_pos >= NUM_LEDS - 1)
        {
            init_led_mode = false;
        }
    }
    else
    {
        for (byte s = 0; s < NUM_STRIPS; s++)
        {
            fade_raw(leds[s], NUM_LEDS, 100);
        }

        for (byte l = 0; l < NUM_LEDS; l++)
        {
            byte strip = (l - active_strip) % NUM_STRIPS;
            leds[strip][l].setHue(hue);
        }
        delay(100);
    }
    inc_strip();
    inc_slow_hue();
}

void led_double_rainbow()
{
    int draw = rand() % 100;
    if (draw > 70)
    {
        for (byte s = 0; s < NUM_STRIPS; s++)
        {
            fade_raw(leds[s], NUM_LEDS, 1);
        }
    }
    byte hue = get_slow_hue();
    byte strip = abs_led_pos / NUM_LEDS;
    byte led = abs_led_pos % NUM_LEDS;

    if (abs_led_pos % 2 == 1)
    {
        leds[strip][led].setHue(hue);
    }
    else
    {
        byte rev_strip = NUM_STRIPS - strip - 1;
        byte rev_led = (NUM_LEDS - led - 2) % NUM_LEDS;
        leds[rev_strip][rev_led].setHue(hue);
    }

    inc_abs_pos();
    inc_slow_hue();
}

void led_sparkle()
{
    int draw = rand() % 100;
    if (draw > 70)
    {
        for (byte s = 0; s < NUM_STRIPS; s++)
        {
            fade_raw(leds[s], NUM_LEDS, 1);
        }
    }
    byte pos = 0;
    byte s = 0;
    if (rand() % 100 > 90)
    {
        s = rand() % NUM_STRIPS;
        pos = rand() % NUM_LEDS;
        leds[s][pos] = CRGB::White;
    }
}

void led_rainbow()
{
    int draw = rand() % 100;
    if (draw > 50)
    {
        for (byte s = 0; s < NUM_STRIPS; s++)
        {
            fade_raw(leds[s], NUM_LEDS, 1);
        }
    }
    byte hue = get_slow_hue();
    leds[abs_led_pos / NUM_LEDS][abs_led_pos % NUM_LEDS].setHue(hue);

    inc_slow_hue();
    inc_abs_pos();
}

void set_rand()
{
    byte pos = rand() % NUM_LEDS;
    byte value = (pos + 50);
    if (pos > 110)
    {
        value = 255;
    }
    byte s = rand() % NUM_STRIPS;
    byte hue = rand() % 256;

    leds[s][pos] = CHSV(hue, 255, value);
}

void led_rainbow_sparkle()
{
    int draw = rand() % 200;
    if (draw > 120)
    {
        for (byte s = 0; s < NUM_STRIPS; s++)
        {
            fade_raw(leds[s], NUM_LEDS, 1);
        }
    }
    if (draw > 190)
    {
        for (byte i = 0; i < 10; i++)
        {
            set_rand();
        }

        draw = rand() % 200;

        if (draw > 180)
        {
            for (byte i = 0; i < 100; i++)
            {
                set_rand();
            }
        }
        else if (draw > 100)
        {
            for (byte i = 0; i < 20; i++)
            {
                set_rand();
            }
        }
    }
    else if (draw > 170)
    {
        set_rand();
    }
}

void loop_leds()
{
    if (led_mode == LED_MODE_LANTERN)
    {
        led_lantern();
    }
    else if (led_mode == LED_MODE_PILLARS)
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
    else if (led_mode == LED_MODE_RAINBOW_SPARKLE)
    {
        led_rainbow_sparkle();
    }
    FastLED.show();
}

void loop()
{
    if (mode == MODE_OFF)
    {
        led_standby();
        delay(3000);
    }
    else
    {
        read_knob();
        loop_leds();
        delay(2);
    }
}
