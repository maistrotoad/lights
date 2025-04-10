#include <Arduino.h>
#include <FastLED.h>
#include <stdint.h>
#include <Encoder.h>

#define KNOB_CLK 12
#define KNOB_DT 11
#define KNOB_SW 10

#define GX A0
#define GY A1
#define GZ A2

#define LED_PIN_A 2
#define LED_PIN_B 3
#define LED_PIN_C 4
#define LED_PIN_D 5
#define NUM_LEDS 144
#define LED_TYPE WS2812
#define COLOR_ORDER GRB

#define MAX_LED_MODE 4
#define MAX_BGRIGHTNESS 50

CRGB leds_x[NUM_LEDS];
CRGB leds_y[NUM_LEDS];

bool is_setting_brightness = false;
bool is_paused = false;
bool is_on = false;

byte brightness = 10;
byte hue = 0;
byte led_mode = 0;
byte led_pos = 0;

Encoder my_enc(KNOB_CLK, KNOB_DT);

int32_t enc_mode_pos = 0;
int32_t enc_brightness_pos = 0;

unsigned long last_button_press = 0;
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

void setup_knob()
{
  pinMode(KNOB_SW, INPUT_PULLUP);
}

void led_standby()
{
  FastLED.setBrightness(10);

  leds_x[0] = CHSV(0, 255, 255);
  leds_y[0] = CHSV(0, 255, 255);

  for (byte i = 1; i < NUM_LEDS; i++)
  {
    leds_x[i] = CRGB::Black;
    leds_y[i] = CRGB::Black;
  }
  FastLED.show();
}

void setup_lights()
{
  FastLED.addLeds<LED_TYPE, LED_PIN_A, COLOR_ORDER>(leds_x, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, LED_PIN_B, COLOR_ORDER>(leds_y, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, LED_PIN_C, COLOR_ORDER>(leds_x, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, LED_PIN_D, COLOR_ORDER>(leds_y, NUM_LEDS).setCorrection(TypicalLEDStrip);

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

void sw()
{
  // Read the button state
  int btn_state = digitalRead(KNOB_SW);

  // If we detect LOW signal, button is pressed
  if (btn_state == LOW)
  {
    // if 50ms have passed since last LOW pulse, it means that the
    // button has been pressed, released and pressed again
    if (millis() - last_button_press > 50)
    {
      Serial.println("Button pressed!");
      if (!is_on)
      {
        is_on = true;
        is_paused = false;
        is_setting_brightness = false;
      }
      else if (is_paused || !is_setting_brightness)
      {
        is_on = false;
        is_paused = false;
      }
      else if (!is_paused)
      {
        is_paused = true;
        is_setting_brightness = false;
      }

      Serial.print("It is ");
      if (is_paused)
      {
        Serial.print("paused ");
      }
      else
      {
        Serial.print("resumed ");
      }

      if (is_on)
      {
        Serial.println("and on");
      }
      else
      {
        Serial.println("and off");
        led_standby();
      }
    }

    // Remember last button press event
    last_button_press = millis();
  }
}

void setup()
{
  delay(10);
  // Setup Serial Monitor
  Serial.begin(38400);

  setup_lights();
  setup_knob();
  setup_acc();

  attachInterrupt(digitalPinToInterrupt(KNOB_SW), sw, CHANGE);
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
  leds_x[led_pos] = CHSV(hue, 255, 255);
  leds_y[led_pos_rev] = CHSV(hue, 255, 255);

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

  leds_x[led_pos] = CHSV(hue, 255, 255);
  leds_x[led_pos_rev] = CHSV(hue_rev, 255, 255);

  leds_y[led_pos_rev] = CHSV(hue, 255, 255);
  leds_y[led_pos] = CHSV(hue_rev, 255, 255);

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
  if (rand() % 100 > 90)
  {
    pos = rand() % NUM_LEDS;
    leds_x[pos] = CRGB::White;
    pos = rand() % NUM_LEDS;
    leds_y[pos] = CRGB::White;
  }
  pos = rand() % NUM_LEDS;
  leds_x[pos] = CRGB::Black;
  pos = rand() % NUM_LEDS;
  leds_y[pos] = CRGB::Black;
}

void led_sparkle_g()
{
  byte pos = 0;
  byte hue_g = gd_acc % 255;

  unsigned int rolls = gd_acc / 10;

  if (rolls > 120)
  {
    rolls = 120;
  }

  for (unsigned int i = 0; i < rolls; i++)
  {
    pos = rand() % NUM_LEDS;
    leds_x[pos] = CHSV(hue_g, 255, 255);
    pos = rand() % NUM_LEDS;
    leds_y[pos] = CHSV(hue_g, 255, 255);
  }

  pos = rand() % NUM_LEDS;
  leds_x[pos] = CRGB::Black;
  pos = rand() % NUM_LEDS;
  leds_y[pos] = CRGB::Black;
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

  byte led_pos_y = NUM_LEDS - led_pos - 1;
  leds_x[led_pos] = CHSV(hue, 255, 255);
  leds_y[led_pos_y] = CHSV(hue, 255, 255);
  for (byte i = 1; i < 10; i++)
  {
    leds_x[get_next_led_pos(led_pos, i)] = CHSV(hue, 255, 255 - i * 25);
    leds_y[get_prev_led_pos(led_pos_y, i)] = CHSV(hue, 255, 255 - i * 25);
  }

  inc_pos();
}

void loop_leds()
{
  if (led_mode == 0)
  {
    led_double_rainbow();
  }
  else if (led_mode == 1)
  {
    led_sparkle();
  }
  else if (led_mode == 2)
  {
    led_rainbow();
  }
  else if (led_mode == 3)
  {
    led_sparkle_g();
  }
  else if (led_mode == MAX_LED_MODE)
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

    if (gd_acc > 10)
    {
      Serial.print("gd: ");
      Serial.print(gd);
      Serial.print(" gd_acc: ");
      Serial.println(gd_acc);
    }

    last_g = millis();
  }

  last_gx = cur_gx;
  last_gy = cur_gy;
  last_gz = cur_gz;
}

void read_knob()
{
  int32_t new_pos = my_enc.read();

  if (is_paused && !is_setting_brightness && new_pos != enc_mode_pos)
  {
    is_setting_brightness = true;
    my_enc.write(enc_brightness_pos);
    new_pos = enc_brightness_pos;
  }

  if (new_pos < 0)
  {
    new_pos = 0;
  }

  if (is_setting_brightness)
  {
    if (new_pos > MAX_BGRIGHTNESS * 10)
    {
      new_pos = MAX_BGRIGHTNESS * 10;
    }
  }
  else
  {
    if (new_pos > MAX_LED_MODE * 10)
    {
      new_pos = MAX_LED_MODE * 10;
    }
  }

  my_enc.write(new_pos);

  if (is_setting_brightness)
  {
    if (round(new_pos / 10) != brightness)
    {
      brightness = round(new_pos / 10);
      FastLED.setBrightness(brightness);
      enc_brightness_pos = new_pos;
    }
  }
  else
  {
    led_mode = round(new_pos / 10);
    enc_mode_pos = new_pos;
  }

  if (millis() - last_print > 1000)
  {
    Serial.print("enc_mode_pos: ");
    Serial.print(enc_mode_pos);
    Serial.print(" enc_brightness_pos: ");
    Serial.println(enc_brightness_pos);

    last_print = millis();
  }
}

void loop()
{
  if (is_on)
  {
    read_knob();
  }
  read_g();
  if (is_on && !is_paused)
  {
    loop_leds();
  }
  delay(5);
}