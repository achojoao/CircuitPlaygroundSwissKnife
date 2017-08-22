#ifndef CLOCKFEATURE_H
#define CLOCKFEATURE_H

#include "Feature.h"
#include "Notes.h"
#include <TimeLib.h>

#define NO_BUTTON       -1
#define CAP_THRESHOLD   100
#define SNOOZE_MINUTES  10

struct clockButton {
  uint8_t capPad[2];
} clockButton[] = {
  { {3,2} },
  { {0,1} },
  { {12, 6} },
  { {9, 10} }
};

class ClockFeature: public Feature {
  
public:
  ClockFeature() {  
    mode = 0; 
    button = NO_BUTTON;
    alarmHour = 7;
    alarmMinute = 0;
    alarmMode = 2;
    snoozeHour = 0;
    snoozeMinute = 0;
    calculateSnooze();    
    setTime(0,0,0,2,8,2017);
    shaken = false;
    lastSecondLight = 0;
  }
  ~ClockFeature() {}

  virtual void setup() {      
    hourColor = CircuitPlayground.strip.Color(random(0, 255), random(0, 255), random(0, 255));
    minuteColor = CircuitPlayground.strip.Color(random(0, 255), random(0, 255), random(0, 255));
  }
  
  virtual void loop() {
    if (second()-lastSecondLight > 10) {
      uint16_t light = CircuitPlayground.lightSensor();
      light = light>30 ? 30 : light<1 ? 1 : light;
      CircuitPlayground.setBrightness(light);
      lastSecondLight = second();
    }    
    if (mode > 0) {
      button = getButtonPress();
      switch (button) {
        case 0:
          switch (mode) {
            case 1:
              adjustTime(3600);
              break;
            case 2:
              alarmHour++;
              if (alarmHour>23) {
                alarmHour = 0;
              }
              calculateSnooze();
              break;
            case 3:
              alarmMode += 1;
              if (alarmMode > 2) {
                alarmMode = 0;
              }
              break;  
          }
          break;
        case 1:
          switch (mode) {
            case 1:
              adjustTime(-3600);
              break;
            case 2:
              alarmHour--;
              if (alarmHour<0) {
                alarmHour = 23;
              }
              calculateSnooze();
              break;
          }
          break;
        case 2:
          switch (mode) {
            case 1:
              adjustTime(-60);
              break;
            case 2:
              alarmMinute--;
              if (alarmMinute<0) {
                alarmMinute = 59;
              }
              calculateSnooze();
              break;
          }
          break;
        case 3:
          switch (mode) {
            case 1:
              adjustTime(60);
              break;
            case 2:
              alarmMinute++;
              if (alarmMinute>59) {
                alarmMinute = 0;
              }
              calculateSnooze();
              break;
          }
          break;
      }
    }
    if (mode < 3) {
      int mHour = hour();
      int mMinute = minute();
      if (mode == 2) {
        mHour = alarmHour;
        mMinute = alarmMinute;
      }
      if (mHour >= 12) {
        mHour = mHour-12;
      }
      uint32_t mHourColor = hourColor;
      uint32_t mMinuteColor = minuteColor;
      if (mode == 2) {
        mHourColor = CircuitPlayground.strip.Color(255, 0, 0);
        mMinuteColor = CircuitPlayground.strip.Color(0, 255, 0);
      }
      for (int i=0; i<4; i++) {      
        if (((mHour & ( 1 << i )) >> i)==1) {
          CircuitPlayground.strip.setPixelColor(4-i, mHourColor);
        } else {
          CircuitPlayground.strip.setPixelColor(4-i, 0x000000);
        }
      }
      for (int i=5; i<10; i++) {
        if (((mMinute & ( 1 << i-5 )) >> i-5)==1) {
          CircuitPlayground.strip.setPixelColor(i, mMinuteColor);
        } else {
          CircuitPlayground.strip.setPixelColor(i, 0x000000);
        }
      }  
      if (((mMinute & ( 1 << 5 )) >> 5)==1) {
        CircuitPlayground.strip.setPixelColor(0, mMinuteColor);
      } else {
        CircuitPlayground.strip.setPixelColor(0, 0x000000);
      }
    } else {
      if (alarmMode == 0) {
        CircuitPlayground.clearPixels();
      } else if (alarmMode == 1) {
        for (int i=0; i<5; i++) {
          CircuitPlayground.strip.setPixelColor(i, CircuitPlayground.strip.Color(0, 0, 255));
          CircuitPlayground.strip.show();
          delay(60);          
        }
      } else {
        for (int i=0; i<5; i++) {
          CircuitPlayground.strip.setPixelColor(i, CircuitPlayground.strip.Color(0, 0, 255));
          CircuitPlayground.strip.setPixelColor(i+5, CircuitPlayground.strip.Color(0, 0, 255));
          CircuitPlayground.strip.show();
          delay(60);
        }
      }
    }
    CircuitPlayground.strip.show();
    if ((alarmMode != 0 && hour() == alarmHour && minute() == alarmMinute) || 
    (alarmMode == 2 && hour() == snoozeHour && minute() == snoozeMinute)) {
      if (!shaken) {
        playAlarm();
      }
    } else if (hour() >= alarmHour && (minute() > alarmMinute || minute() < alarmMinute)) {
      shaken = false;
    }
    if (mode > 0 && mode < 3) {
      delay(200);
      CircuitPlayground.clearPixels();
      delay(200);
    } else if (mode == 3) {
      delay(200);
    }
  }

  virtual void startPressed() {}

  virtual void modePressed() {
    CircuitPlayground.playTone(2200, 50);
    mode += 1;
    if (mode > 3) {
      mode = 0;
    }
  }

  virtual void tapDetected() {
  }

  uint8_t getButtonPress() {
    for (int b=0; b<4; b++) {
      for (int p=0; p<2; p++) {
        if (CircuitPlayground.readCap(clockButton[b].capPad[p]) > CAP_THRESHOLD) {
          return b;
        }
      }
    }
    return NO_BUTTON;
  }

  void calculateSnooze() {
    snoozeHour = alarmHour;
    snoozeMinute = alarmMinute + SNOOZE_MINUTES;
    if (snoozeMinute>=60) {
      snoozeHour++;
      snoozeMinute = snoozeMinute - 60;
    }
  }

  void playAlarm() {

    int numNotes = 61;  
    int melody[] = {NOTE_E5,0,NOTE_F5,NOTE_G5,0,NOTE_C6,0,NOTE_D5,0,NOTE_E5,NOTE_F5,0,NOTE_G5,0,NOTE_A5,NOTE_B5,0,NOTE_F6,0,NOTE_A5,0,NOTE_B5,NOTE_C6,NOTE_D6,NOTE_E6,NOTE_E5,0,NOTE_F5,NOTE_G5,0,NOTE_C6,0,NOTE_D6,0,NOTE_E6,NOTE_F6,0,NOTE_G5,0,NOTE_G5,NOTE_E6,0,NOTE_D6,0,NOTE_G5,NOTE_E6,0,NOTE_D6,0,NOTE_G5,NOTE_E6,0,NOTE_D6,0,NOTE_G5,NOTE_F6,0,NOTE_E6,0,NOTE_D6,NOTE_C6 };
 
  int noteDurations[] = {8,16,16,16,16,2,8,8,16,16,2,4,8,16,16,16,16,2,8,8,16,16,4,4,4,8,16,16,16,16,2,8,8,16,16,2,4,8,16,16,8,8,8,16,16,8,8,8,16,16,8,8,8,16,16,8,8,8,16,16,2};

    for (int thisNote = 0; thisNote < numNotes && !shaken; thisNote++) { // play notes of the melody
      // to calculate the note duration, take one second divided by the note type.
      //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
      int noteDuration = 750 / noteDurations[thisNote];
      CircuitPlayground.playTone(melody[thisNote], noteDuration);
 
      // to distinguish the notes, set a minimum time between them.
      //   the note's duration + 30% seems to work well:
      int pauseBetweenNotes = noteDuration * 1.30;
      
      // Compute total acceleration
        X = 0;
        Y = 0;
        Z = 0;
        for (int i=0; i<10; i++) {
          X += CircuitPlayground.motionX();
          Y += CircuitPlayground.motionY();
          Z += CircuitPlayground.motionZ();
          delay(1);
        }
        X /= 10;
        Y /= 10;
        Z /= 10;  
        totalAccel = sqrt(X*X + Y*Y + Z*Z);
        if (totalAccel > 15) {
          shaken = true;
        }
        delay(pauseBetweenNotes);
    }
}
 private:
  int mode, alarmMode;
  int8_t button;
  uint32_t hourColor, minuteColor;
  int alarmHour, alarmMinute, snoozeHour, snoozeMinute;
  boolean shaken;
  float X, Y, Z, totalAccel;
  uint16_t lastSecondLight;
};

#endif

