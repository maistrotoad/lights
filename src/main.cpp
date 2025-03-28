#include <Arduino.h>
#include <FastLED.h>
#include <stdint.h>

#define KNOB_CLK 2
#define KNOB_DT 3
#define KNOB_SW 4

#define GX A1
#define GY A2
#define GZ A3

#define LED_PIN_A 5
#define LED_PIN_B 6
#define LED_PIN_C 10
#define LED_PIN_D 11
#define NUM_LEDS 144
#define LED_TYPE WS2812
#define COLOR_ORDER GRB

#define MAX_LED_MODE 3

CRGB leds_x[NUM_LEDS];
CRGB leds_y[NUM_LEDS];

bool is_paused = false;
bool is_on = false;

byte brightness = 10;
byte hue = 0;
byte led_mode = 0;
byte led_pos = 0;

unsigned long last_button_press = 0;

int cur_gx;
int last_gx;
int cur_gy;
int last_gy;
int cur_gz;
int last_gz;
unsigned int gd = 0;
unsigned long last_g = 0;
unsigned int gd_acc = 0;

unsigned long last_dt_up = 0;
unsigned long last_clk_up = 0;

void wait_for_serial_connection()
{
  uint32_t timeout_end = millis() + 2000;
  Serial.begin(9600);
  while (!Serial && timeout_end > millis())
  {
  } // wait until the connection to the PC is established
}

void setup_knob()
{
  // Set encoder pins as inputs
  pinMode(KNOB_CLK, INPUT_PULLUP);
  pinMode(KNOB_DT, INPUT_PULLUP);
  pinMode(KNOB_SW, INPUT_PULLUP);

  // Setup Serial Monitor
  Serial.begin(9600);
}

void setup_lights()
{
  FastLED.addLeds<LED_TYPE, LED_PIN_A, COLOR_ORDER>(leds_x, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, LED_PIN_B, COLOR_ORDER>(leds_y, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, LED_PIN_C, COLOR_ORDER>(leds_x, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, LED_PIN_D, COLOR_ORDER>(leds_y, NUM_LEDS).setCorrection(TypicalLEDStrip);

  delay(100);
  FastLED.setBrightness(brightness);

  leds_x[0] = CHSV(0, 255, 255);
  leds_y[0] = CHSV(0, 255, 255);
  FastLED.show();
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

void clk()
{
  int clk_v = digitalRead(KNOB_CLK);
  if (clk_v == 1)
  {
    last_clk_up = millis();
    if (last_clk_up - last_dt_up < 60)
    {
      Serial.println("CCW / decrement");
      if (!is_paused && brightness >= 10)
      {
        brightness -= 5;
        FastLED.setBrightness(brightness);
        Serial.println(brightness);
      }
      else if (is_paused && led_mode > 0)
      {
        led_mode--;
      }
      is_paused = false;
    }
  }
}

void dt()
{
  int dt_v = digitalRead(KNOB_DT);
  if (dt_v == 1)
  {
    last_dt_up = millis();
    if (last_dt_up - last_clk_up < 60)
    {
      Serial.println("CW / increment");
      if (!is_paused && brightness <= 95)
      {
        brightness += 5;
        FastLED.setBrightness(brightness);
        Serial.println(brightness);
      }
      else if (is_paused && led_mode < MAX_LED_MODE)
      {
        led_mode++;
      }
      is_paused = false;
    }
  }
}

void setup()
{
  wait_for_serial_connection(); // Optional, but seems to help Teensy out a lot.
  setup_lights();
  setup_knob();
  setup_acc();

  attachInterrupt(digitalPinToInterrupt(KNOB_CLK), clk, RISING);
  attachInterrupt(digitalPinToInterrupt(KNOB_DT), dt, RISING);
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
  int led_pos_y = NUM_LEDS - led_pos - 1;
  leds_x[led_pos] = CHSV(hue, 255, 255);
  leds_y[led_pos_y] = CHSV(hue, 255, 255);

  hue++;
  if (hue > 255)
  {
    hue = 0;
  }

  inc_pos();
}

void led_sparkle()
{
  int pos = 0;
  pos = rand() % NUM_LEDS;
  leds_x[pos] = CRGB::White;
  pos = rand() % NUM_LEDS;
  leds_y[pos] = CRGB::White;

  for (int i = 0; i < 2; i++)
  {
    pos = rand() % NUM_LEDS;
    leds_x[pos] = CRGB::Black;
    pos = rand() % NUM_LEDS;
    leds_y[pos] = CRGB::Black;
  }
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

void led_standby()
{
  leds_x[0] = CHSV(0, 255, 255);
  leds_y[0] = CHSV(0, 255, 255);

  for (byte i = 1; i < NUM_LEDS; i++)
  {
    leds_x[i] = CRGB::Black;
    leds_y[i] = CRGB::Black;
  }
  FastLED.show();
}

void loop_leds()
{
  if (led_mode == 0)
  {
    led_rainbow();
  }
  else if (led_mode == 1)
  {
    led_sparkle();
  }
  else if (led_mode == 2)
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
      }
      else if (is_paused)
      {
        is_on = false;
      }
      else
      {
        is_paused = true;
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

void loop()
{
  read_knob();
  read_g();
  if (is_on && !is_paused)
  {
    loop_leds();
  }
  delay(1);
}