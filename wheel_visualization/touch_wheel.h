#ifndef TOUCH_WHEEL_H
#define TOUCH_WHEEL_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"

class TouchWheel {
private:
  TFT_eSPI* tft;
  
  // Touch sensing
  int baseline1, baseline2, baseline3;
  bool calibrated;
  int calibrationCount;
  float lastAngle;
  
  // Display state
  int lastSegmentDrawn;
  bool wheelInitialized;
  int lastPointX, lastPointY;
  bool wasShowingAngle;
  int lastSegmentForTouch;
  float lastDisplayedAngle;
  int lastDisplayedSegment;
  bool lastTouchedState;
  bool needsRedraw;
  int lastDev1, lastDev2, lastDev3;
  
  // LED state
  int currentLedPosition;
  unsigned long lastTouchTime;
  
  void drawSegment(int cx, int cy, int innerR, int outerR, 
                   float startAngle, float endAngle, uint16_t color);
  void drawStaticInterface();
  void updateSensorDisplay(int dev1, int dev2, int dev3);
  void drawBar(int x, int y, int value);
  void updateLEDs(int position);
  void turnOffAllLEDs();
  
public:
  TouchWheel(TFT_eSPI* display);
  void init();
  void calibrate(int touch1, int touch2, int touch3);
  bool isCalibrated() { return calibrated; }
  float calculateAngle(int dev1, int dev2, int dev3);
  int angleToLedPosition(float angle);
  int angleToSegment(float angle);
  void draw(float angle, bool touched, int segment);
  void drawCenterButton(bool pressed);
  void update(int touch1, int touch2, int touch3, bool selectPressed);
  void reset();
};

#endif // TOUCH_WHEEL_H