#include "FastLED.h"
#define NUM_STRIPS 1
#define NUM_LEDS_PER_STRIP 120
CRGB leds[NUM_STRIPS][NUM_LEDS_PER_STRIP];

int val = 0;
int cutoffIndex = 0;
int prevCutoffIndex = 0;
int threshold = 10;
int brightness = 100;
int colorWeight = -1;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  FastLED.addLeds<NEOPIXEL, 10>(leds[0], NUM_LEDS_PER_STRIP);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.write(1);
}

void loop() {
  /************************************ LINE *********************************************/
    if (Serial.available()) {
    val = Serial.read();
    prevCutoffIndex = cutoffIndex;
    cutoffIndex = (int)(NUM_LEDS_PER_STRIP - NUM_LEDS_PER_STRIP * threshold / (val+0.0)); //obtained from the formula (total_leds-i)/total_leds * val < threshold
    if (cutoffIndex > (NUM_LEDS_PER_STRIP - 1)) cutoffIndex = (NUM_LEDS_PER_STRIP - 1); //prevent index out of range
    if (cutoffIndex < 0) cutoffIndex = 0;
    }

    if (cutoffIndex > prevCutoffIndex) { //grow right
      for(int i = prevCutoffIndex+1; i <= cutoffIndex; i++) {
        leds[0][i] = CRGB(brightness - i / (NUM_LEDS_PER_STRIP - 1.0) * brightness, i / (NUM_LEDS_PER_STRIP - 1.0) * brightness, 0);
    }

    } else if (cutoffIndex < prevCutoffIndex){ //shrink left
     for(int i = prevCutoffIndex; i > cutoffIndex; i--) {
        leds[0][i] = CRGB::Black;
     }
    }
    FastLED.show();
  
  /************************************* EXPLOSIVE *********************************************/ /*
    if (Serial.available()) {
      val = Serial.read();
      colorWeight++;
    }

    if (colorWeight >= 0 && colorWeight < 1000) {
      for (int i = 1; i <= NUM_LEDS_PER_STRIP / 2; i++) {
        leds[0][i - 1] = leds[0][NUM_LEDS_PER_STRIP - i] = CRGB((1000 - colorWeight) / 1000.0 * i * 2.0 / NUM_LEDS_PER_STRIP * val, colorWeight / 1000.0 * i * 2.0 / NUM_LEDS_PER_STRIP * val, 0);
      }
    } else if (colorWeight >= 1000 && colorWeight < 2000) {
      for (int i = 1; i <= NUM_LEDS_PER_STRIP / 2; i++) {
        leds[0][i - 1] = leds[0][NUM_LEDS_PER_STRIP - i] = CRGB(0, (2000 - colorWeight) / 1000.0 * i * 2.0 / NUM_LEDS_PER_STRIP * val, (colorWeight - 1000.0) / 1000 * i * 2.0 / NUM_LEDS_PER_STRIP * val);
      }
    } else if (colorWeight >= 2000 && colorWeight < 3000) {
      for (int i = 1; i <= NUM_LEDS_PER_STRIP / 2; i++) {
        leds[0][i - 1] = leds[0][NUM_LEDS_PER_STRIP - i] = CRGB((colorWeight - 2000.0) / 1000 * i * 2.0 / NUM_LEDS_PER_STRIP * val, 0 , (3000 - colorWeight) / 1000.0 * i * 2.0 / NUM_LEDS_PER_STRIP * val);
      }
    } else if (colorWeight >= 3000 && colorWeight < 4000) {
      for (int i = 1; i <= NUM_LEDS_PER_STRIP / 2; i++) {
        leds[0][i - 1] = leds[0][NUM_LEDS_PER_STRIP - i] = CRGB((4000 - colorWeight) / 1000.0 * i * 2.0 / NUM_LEDS_PER_STRIP * val, 0 , (colorWeight - 3000.0) / 1000 * i * 2.0 / NUM_LEDS_PER_STRIP * val);
      }
    } else if (colorWeight >= 4000 && colorWeight < 5000) {
      for (int i = 1; i <= NUM_LEDS_PER_STRIP / 2; i++) {
        leds[0][i - 1] = leds[0][NUM_LEDS_PER_STRIP - i] = CRGB(0, (colorWeight - 4000.0) / 1000 * i * 2.0 / NUM_LEDS_PER_STRIP * val , (5000 - colorWeight) / 1000.0 * i * 2.0 / NUM_LEDS_PER_STRIP * val);
      }
    } else if (colorWeight >= 5000 && colorWeight < 6000) {
      for (int i = 1; i <= NUM_LEDS_PER_STRIP / 2; i++) {
        leds[0][i - 1] = leds[0][NUM_LEDS_PER_STRIP - i] = CRGB((colorWeight - 5000.0) / 1000 * i * 2.0 / NUM_LEDS_PER_STRIP * val, (6000 - colorWeight) / 1000.0 * i * 2.0 / NUM_LEDS_PER_STRIP * val , 0);
      }
    } else {
      colorWeight %= 6000;
    }


    FastLED.show();
  */
  
  /************************************* RANDOM *********************************************/ /*
  for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
    leds[0][i] = CRGB(random(0, brightness + 1) / (brightness + 0.0) * brightness, random(0, brightness + 1) / (brightness + 0.0) * brightness, random(0, brightness + 1) / (brightness + 0.0) * brightness);
  }
  FastLED.show();
*/
}

