#include <Adafruit_NeoPixel.h>

#define LED_STRAND_PIN 6
#define NUM_LEDS 250

#define YELLOW_BUTTON_PIN 9
#define WHITE_BUTTON_PIN 10
#define RED_BUTTON_PIN 7
#define GREEN_BUTTON_PIN 11
#define BLUE_BUTTON_PIN 8

#define POTENTIOMETER_PIN 6


// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_STRAND_PIN, NEO_RGB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

enum mode {
  NONE = 0,
  TWINKLE,
  RAINBOW,
  RED,
  GREEN,
  BLUE
};

void setup() {
  pinMode(WHITE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(YELLOW_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RED_BUTTON_PIN, INPUT_PULLUP);
  pinMode(GREEN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BLUE_BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(9600);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

mode read_buttons(){
    if (0 == digitalRead(YELLOW_BUTTON_PIN)) {
      return TWINKLE;
    }
    if (0 == digitalRead(WHITE_BUTTON_PIN)) {
      return RAINBOW;
    }
    if (0 == digitalRead(RED_BUTTON_PIN)) {
      return RED;
    }
    if (0 == digitalRead(GREEN_BUTTON_PIN)) {
      return GREEN;
    }
    if (0 == digitalRead(BLUE_BUTTON_PIN)) {
      return BLUE;
    }
    return NONE;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t wheel(byte wheel_pos) {
  wheel_pos = 255 - wheel_pos;
  if(wheel_pos < 85) {
    return strip.Color(255 - wheel_pos * 3, 0, wheel_pos * 3);
  }
  if(wheel_pos < 170) {
    wheel_pos -= 85;
    return strip.Color(0, wheel_pos * 3, 255 - wheel_pos * 3);
  }
  wheel_pos -= 170;
  return strip.Color(wheel_pos * 3, 255 - wheel_pos * 3, 0);
}

// Position of next LED to turn on for color chase.
uint16_t chase_position = 0;
void chase_step(uint32_t color) {
  if (chase_position >= NUM_LEDS * 2) {
    chase_position = 0;
  }
  if (chase_position >= NUM_LEDS) {
    // Fully colored strip, start turning them off again.
    strip.setPixelColor(chase_position - NUM_LEDS, strip.Color(0, 0, 0));
  } else {
    strip.setPixelColor(chase_position, color);
  }
  ++chase_position;
}

// Position in rainbow iteration.
uint16_t rainbow_position = 0;
uint16_t rainbow_color_position = 0;
void rainbow_step() {
  // 7 LEDs race along the strip in rainbow pattern.
  uint16_t high = rainbow_position;
  uint16_t low = 0;
  const uint16_t num_rainbow_pixels = 20;
  if (rainbow_position > num_rainbow_pixels) {
    low = rainbow_position - num_rainbow_pixels;
  }

  // Clear trailing end.
  if (low > 0) {
    strip.setPixelColor(low - 1, strip.Color(0, 0, 0));
  }

   for(uint16_t i = low; i < high; ++i) {
     if (i >= NUM_LEDS) {
       continue;
     }
     uint32_t c = wheel(((i * 256 / NUM_LEDS) + rainbow_color_position) & 255);
     strip.setPixelColor(i, c);
  }

  if (rainbow_position++ >= (NUM_LEDS + num_rainbow_pixels)) {
    rainbow_position = 0;
  }
  if (rainbow_color_position++ >= 256*5) {
    rainbow_color_position = 0;
  }
}

// Position in rainbow iteration.
uint16_t twinkle_position = 0;
void twinkle_step() {
  for(uint16_t i = 0; i < NUM_LEDS; ++i) {
    strip.setPixelColor(i, wheel(((i * 256 / NUM_LEDS) + twinkle_position) & 255));
  }
  if (twinkle_position++ >= 5 * 256) {
    twinkle_position = 0;
  }
}

mode current_mode = TWINKLE;
void loop() {
  mode new_mode = read_buttons();
  if (new_mode != NONE) {
    // Reset everything.
    chase_position = 0;
    rainbow_position = 0;
    twinkle_position = 0;
    // Turn everything off if previously twinkling, or if same button press.
    // Otherwise hard to see new pattern.
    if (current_mode == TWINKLE || current_mode == new_mode) {
      for (int i = 0; i < NUM_LEDS; ++i) {
        strip.setPixelColor(i, strip.Color(0, 0, 0));
      }
    }
    current_mode = new_mode;
  }

  switch(current_mode) {
    case TWINKLE:
      twinkle_step();
      break;
    case RAINBOW:
      rainbow_step();
      break;
    case RED:
      chase_step(strip.Color(128, 0, 0));
      break;
    case GREEN:
      chase_step(strip.Color(0, 128, 0));
      break;
    case BLUE:
      chase_step(strip.Color(0, 0, 128));
      break;
  }

  strip.show();

  // Delay is based on potentiometer input.
  // Range is [0, 1023).
  // Scaling based on just messing about with values.
  uint32_t val = analogRead(POTENTIOMETER_PIN);
  uint32_t delay_ms = 30 + val / 20;
  delay(delay_ms);
}
