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

#define GX A0
#define GY A1
#define GZ A2

#define LED_PIN_A 8
#define LED_PIN_B 14
#define LED_PIN_C 17
#define LED_PIN_D 20

#define NUM_LEDS 144
#define NUM_STRIPS 4

#define COLOR_ORDER GRB

#define LED_MODE_DOUBLE_RAINBOW 0
#define LED_MODE_SPARKLE 1
#define LED_MODE_RAINBOW 2
#define LED_MODE_SPARKLE_G 3
#define LED_MODE_PAINTER_G 4
#define MAX_LED_MODE 4

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

Encoder my_enc(KNOB_DT, KNOB_CLK);

int32_t enc_led_mode_pos = 0;
int32_t enc_brightness_pos = MIN_BRIGHTNESS * 10;

unsigned long last_button_press = 0;
byte fast_button_count = 3;

unsigned long last_print = 0;

int cur_gx;
int last_gx;
int cur_gy;
int last_gy;
int cur_gz;
int last_gz;
unsigned int gd = 0;
unsigned long last_g = 0;
unsigned int gd_acc = 0;

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

void setup_acc()
{
    last_gx = analogRead(GX);
    last_gy = analogRead(GY);
    last_gz = analogRead(GZ);
    cur_gx = last_gx;
    cur_gy = last_gy;
    cur_gz = last_gz;
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
            // Serial.println("Button pressed!");
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

            // Serial.print("It is ");
            if (mode == MODE_OFF)
            {
                // Serial.println("off");
                led_standby();
            }
            // else
            // {
            //     Serial.println("on");
            // }
        }
        else
        {
            fast_button_count++;
            // Serial.print("Fast button press: ");
            // Serial.println(fast_button_count);
            if (fast_button_count >= 3)
            {
                mode = MODE_OFF;
                led_standby();
                // Serial.println("Powering off");
                fast_button_count = 0;
            }
        }
        // Serial.print("Mode: ");
        // Serial.println(mode);
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
    // Setup Serial Monitor
    // Serial.begin(38400);

    setup_lights();
    setup_knob();
    // setup_acc();
}

void inc_pos()
{
    led_pos++;
    if (led_pos >= NUM_LEDS)
    {
        led_pos = 0;
    }
}

void led_rainbow()
{
    byte led_pos_rev = NUM_LEDS - led_pos - 1;
    for (byte s = 0; s < NUM_STRIPS; s++)
    {
        if (s % 1 == 0)
        {
            leds[s][led_pos] = CHSV(hue, 255, 255);
        }
        else
        {
            leds[s][led_pos_rev] = CHSV(hue, 255, 255);
        }
    }

    hue++;
    if (hue > 255)
    {
        hue = 0;
    }

    inc_pos();
}

void led_double_rainbow()
{
    byte led_pos_rev = NUM_LEDS - led_pos - 1;
    byte hue_rev = hue + 128;

    for (byte s = 0; s < NUM_STRIPS; s++)
    {
        if (s % 1 == 0)
        {
            leds[s][led_pos] = CHSV(hue, 255, 255);
            leds[s][led_pos_rev] = CHSV(hue_rev, 255, 255);
        }
        else
        {
            leds[s][led_pos_rev] = CHSV(hue, 255, 255);
            leds[s][led_pos] = CHSV(hue_rev, 255, 255);
        }
    }

    hue++;
    if (hue > 255)
    {
        hue = 0;
    }

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

void led_sparkle_g()
{
    byte pos = 0;
    byte s = 0;
    byte hue_g = gd_acc % 255;

    unsigned int rolls = gd_acc / 10;

    if (rolls > 120)
    {
        rolls = 120;
    }

    for (unsigned int i = 0; i < rolls; i++)
    {
        s = rand() % NUM_STRIPS;
        pos = rand() % NUM_LEDS;
        leds[s][pos] = CHSV(hue_g, 255, 255);
    }

    s = rand() % NUM_STRIPS;
    pos = rand() % NUM_LEDS;
    leds[s][pos] = CRGB::Black;
    s = rand() % NUM_STRIPS;
    pos = rand() % NUM_LEDS;
    leds[s][pos] = CRGB::Black;
}

byte get_next_led_pos(byte p, byte n)
{
    return (p + n) % NUM_LEDS;
}

byte get_prev_led_pos(byte p, byte n)
{
    return (p - n) % NUM_LEDS;
}

void led_painter_g()
{
    hue = ((gd_acc / 10) + (rand() % 5)) % 255;
    byte led_pos_rev = NUM_LEDS - led_pos - 1;

    for (byte s = 0; s < NUM_STRIPS; s++)
    {
        if (s % 1 == 0)
        {
            leds[s][led_pos] = CHSV(hue, 255, 255);
        }
        else
        {
            leds[s][led_pos_rev] = CHSV(hue, 255, 255);
        }
    }
    for (byte s = 0; s < NUM_STRIPS; s++)
    {
        for (byte i = 1; i < 10; i++)
        {
            if (s % 1 == 0)
            {
                leds[s][get_next_led_pos(led_pos, i)] = CHSV(hue, 255, 255 - i * 25);
            }
            else
            {
                leds[s][get_prev_led_pos(led_pos_rev, i)] = CHSV(hue, 255, 255 - i * 25);
            }
        }
    }
    inc_pos();
}

void loop_leds()
{
    if (led_mode == LED_MODE_DOUBLE_RAINBOW)
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
    else if (led_mode == LED_MODE_SPARKLE_G)
    {
        led_sparkle_g();
    }
    else if (led_mode == LED_MODE_PAINTER_G)
    {
        led_painter_g();
    }
    FastLED.show();
}

void read_g()
{
    cur_gx = analogRead(GX);
    cur_gy = analogRead(GY);
    cur_gz = analogRead(GZ);

    gd = abs(last_gx - cur_gx) + abs(last_gy - cur_gy) + abs(last_gz - cur_gz);

    if (gd <= 10)
    {
        gd = 0;
    }

    gd_acc += gd;

    if (millis() - last_g > 500)
    {
        gd_acc *= 0.9;

        // if (gd_acc > 10)
        // {
        //     Serial.print("gd: ");
        //     Serial.print(gd);
        //     Serial.print(" gd_acc: ");
        //     Serial.println(gd_acc);
        // }

        last_g = millis();
    }

    last_gx = cur_gx;
    last_gy = cur_gy;
    last_gz = cur_gz;
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

    if (millis() - last_print > 1000)
    {
        // Serial.print("enc_mode_pos: ");
        // Serial.print(enc_led_mode_pos);
        // Serial.print(" enc_brightness_pos: ");
        // Serial.println(enc_brightness_pos);

        last_print = millis();
    }
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
        // read_g();
        loop_leds();
        delay(10);
    }
}