#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "touch_wheel_tracker.h"

// Screen states
enum ScreenState {
  WHEEL_SCREEN,
  MENU_SCREEN,
  MINI_GAMES_SCREEN,
  CREDITS_SCREEN
};

// Menu item structure
struct MenuItem {
  const char* name;
  ScreenState targetScreen;
};

class MenuSystem {
private:
  TFT_eSPI* tft;
  MenuItem* items;
  int itemCount;
  int selectedItem;
  int scrollOffset;
  int maxVisibleItems;
  unsigned long lastNavTime;
  bool selectPressed;
  TouchWheelTracker* wheelTracker;
  
public:
  MenuSystem(TFT_eSPI* display);
  void init();
  void draw();
  void drawSelective(int oldSelection, int newSelection);
  void navigateUp();
  void navigateDown();
  ScreenState select();
  void handleInput(int touch1, int touch2, int touch3, bool selectButton);
  void setBaselines(int b1, int b2, int b3);
  int getSelectedItem() { return selectedItem; }
  TouchWheelTracker* getWheelTracker() { return wheelTracker; }
};

#endif // MENU_H