#include "credits_screen.h"

CreditsScreen::CreditsScreen(TFT_eSPI* display) {
  tft = display;
}

void CreditsScreen::init() {
  // Nothing special to initialize
}

void CreditsScreen::draw() {
  // Clear screen
  tft->fillScreen(TFT_BLACK);
  
  // Title
  tft->setTextSize(2);
  tft->setTextColor(COLOR_TEXT);
  const char* title = "CREDITS";
  int titleWidth = strlen(title) * 12;
  tft->setCursor((SCREEN_WIDTH - titleWidth) / 2, 40);
  tft->println(title);
  
  // Credits text
  tft->setTextSize(1);
  tft->setTextColor(COLOR_SEGMENT_ACTIVE);
  
  const char* line1 = "All credit to Fourfold.";
  int textWidth = strlen(line1) * 6;
  tft->setCursor((SCREEN_WIDTH - textWidth) / 2, 120);
  tft->println(line1);
  
  const char* line2 = "Visit fourfold.co";
  textWidth = strlen(line2) * 6;
  tft->setCursor((SCREEN_WIDTH - textWidth) / 2, 140);
  tft->println(line2);
  
  const char* line3 = "for more info.";
  textWidth = strlen(line3) * 6;
  tft->setCursor((SCREEN_WIDTH - textWidth) / 2, 160);
  tft->println(line3);
  
  // Instructions
  tft->setTextSize(1);
  tft->setTextColor(COLOR_TEXT);
  const char* instruction = "Press SELECT for menu";
  textWidth = strlen(instruction) * 6;
  tft->setCursor((SCREEN_WIDTH - textWidth) / 2, 240);
  tft->println(instruction);
}