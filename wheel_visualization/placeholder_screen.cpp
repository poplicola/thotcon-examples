#include "placeholder_screen.h"

PlaceholderScreen::PlaceholderScreen(TFT_eSPI* display, const String& screenTitle) {
  tft = display;
  title = screenTitle;
  displayInitialized = false;
}

void PlaceholderScreen::init() {
  displayInitialized = false;
}

void PlaceholderScreen::draw() {
  if (!displayInitialized) {
    // Clear screen
    tft->fillScreen(TFT_BLACK);
    
    // Draw title
    tft->setTextSize(2);
    tft->setTextColor(COLOR_TEXT);
    int titleWidth = title.length() * 12;
    tft->setCursor((240 - titleWidth) / 2, 50);
    tft->println(title);
    
    // Draw "Coming Soon" message
    tft->setTextSize(3);
    tft->setTextColor(COLOR_SEGMENT_ACTIVE); // Orange
    tft->setCursor(30, 120);
    tft->println("Coming Soon");
    
    // Draw subtitle
    tft->setTextSize(1);
    tft->setTextColor(COLOR_TEXT);
    tft->setCursor(70, 170);
    tft->println("This feature is under");
    tft->setCursor(85, 185);
    tft->println("development");
    
    // Draw instructions
    tft->setTextSize(1);
    tft->setTextColor(COLOR_TEXT);
    tft->setCursor(60, 230);
    tft->println("Press SELECT to return");
    tft->setCursor(80, 245);
    tft->println("to menu");
    
    displayInitialized = true;
  }
}