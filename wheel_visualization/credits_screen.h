#ifndef CREDITS_SCREEN_H
#define CREDITS_SCREEN_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"

class CreditsScreen {
private:
  TFT_eSPI* tft;
  
public:
  CreditsScreen(TFT_eSPI* display);
  void init();
  void draw();
};

#endif // CREDITS_SCREEN_H