#ifndef FLASHLIGHTFEATURE_H
#define FLASHLIGHTFEATURE_H

#include "Feature.h"

#define CAP_THRESHOLD   100

static int color[][3] = { {255,255,255}, {255,0,0}, {0,255,0}, {0,0,255}, {255,255,0}, {0,255,255}, {255,0,255},
                          {255,128,0}, {255,0,128}, {0,255,128}, {128,255,0}, {0,128,255}, {128,0,255} };


struct dimmButton {
  uint8_t capPad[2];
} dimmButton[] = {
  { {9,10} }
};

class FlashLightFeature: public Feature {
  
public:
  FlashLightFeature() {
    mode = 0;
    bright = 255;
  }
  ~FlashLightFeature() {}

  virtual void setup() {
    
  }

  virtual void loop() {
    CircuitPlayground.setBrightness(bright);
    for(int i=0; i<10; i++) {
      CircuitPlayground.strip.setPixelColor(i, color[mode][0], color[mode][1], color[mode][2]);
    }
    CircuitPlayground.strip.show();
    if (getDimmButtonPress()) {
        bright += 10;
        if (bright > 255) {
          bright = 1;
        }
    }
  }
  
  virtual void startPressed() {}

  virtual void modePressed() {
    mode += 1;
    if (mode == sizeof(color)/sizeof(color[0])) {
      mode = 0;
    }
  }

  virtual void tapDetected() {}

  boolean getDimmButtonPress() {    
    for (int p=0; p<2; p++) {
      if (CircuitPlayground.readCap(dimmButton[0].capPad[p]) > CAP_THRESHOLD) {
        return true;
      }
    }    
    return false;
  }
  
private:
  int mode;
  int bright;
};



#endif
