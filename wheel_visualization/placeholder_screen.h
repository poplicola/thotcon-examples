#ifndef PLACEHOLDER_SCREEN_H
#define PLACEHOLDER_SCREEN_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"

class PlaceholderScreen {
private:
  TFT_eSPI* tft;
  String title;
  bool displayInitialized;
  
public:
  PlaceholderScreen(TFT_eSPI* display, const String& screenTitle);
  
  void init();
  void draw();
};

#endif // PLACEHOLDER_SCREEN_H