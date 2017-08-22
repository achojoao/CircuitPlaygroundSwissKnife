#include <Adafruit_CircuitPlayground.h>
#include <Wire.h>
#include <SPI.h>
#include "Adafruit_SleepyDog.h"

#include "Feature.h"
#include "DiceFeature.h"
#include "ClockFeature.h"
#include "SimonFeature.h"
#include "FlashLightFeature.h"

DiceFeature diceFeature;
ClockFeature clockFeature;
SimonFeature simonFeature;
FlashLightFeature flashLightFeature;

int currentFeature = 0;
Feature* features[] = {
  &clockFeature,
  &flashLightFeature,
  &diceFeature,
  &simonFeature
};

void setup() {
  
  Serial.begin(115200);
  Serial.println("Circuit Playground Swiss Knife!");
  
  CircuitPlayground.begin();
  CircuitPlayground.setAccelRange(LIS3DH_RANGE_8_G);
  CircuitPlayground.setAccelTap(2, TAP_THRESHOLD);
  
  // Setup tap detection and callback function
  attachInterrupt(digitalPinToInterrupt(7), tapCallback, RISING);
  
  // Seed the random function with noise
  int seed = 0;  
  seed += analogRead(12);
  seed += analogRead(7);
  seed += analogRead(9);
  seed += analogRead(10);
  randomSeed(seed);

  rainbowCycle();
  
  //Setup all features
  for (int i = 0; i < (sizeof(features)/sizeof(Feature*)); i++) {
      features[i]->setup();
  }

  
}

void loop() {
  while (!CircuitPlayground.slideSwitch()) {
    CircuitPlayground.clearPixels();
    Watchdog.sleep(1000);
  }

  bool leftFirst = CircuitPlayground.leftButton();
  bool rightFirst = CircuitPlayground.rightButton();
  delay(10);

  features[currentFeature]->loop();

  bool leftSecond = CircuitPlayground.leftButton();
  bool rightSecond = CircuitPlayground.rightButton();

  if (leftFirst && !leftSecond) {
    CircuitPlayground.clearPixels();
    CircuitPlayground.setBrightness(30);
    currentFeature += 1;
    if (currentFeature >= (sizeof(features)/sizeof(Feature*))) {
      currentFeature = 0;
    }
    features[currentFeature]->startPressed();
    Serial.print("Changed to feature: "); Serial.println(currentFeature, DEC);
  }

  if (rightFirst && !rightSecond) {
    features[currentFeature]->modePressed();
  }
}

void tapCallback() {
  features[currentFeature]->tapDetected();
}

void rainbowCycle() {
  for (int i=0; i<500; i++) {
    // Loop through each pixel and set it to an incremental color wheel value.
    for(int j=0; j<10; j++) {
      CircuitPlayground.strip.setPixelColor(j, CircuitPlayground.colorWheel(((j * 256 / 10) + i) & 255));
    }
    // Show all the pixels.
    CircuitPlayground.strip.show();
    delay(10);
  }
}

