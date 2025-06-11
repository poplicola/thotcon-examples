/*
 * ESP32 Touch Wheel Badge - Modular Version
 * 
 * This is the refactored version using separated components.
 * Rename this to wheel_visualization.ino after backing up the original.
 */

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "menu.h"
#include "audio.h"
#include "sprites.h"
#include "placeholder_screen.h"
#include "credits_screen.h"

// Global objects
TFT_eSPI tft = TFT_eSPI();
MenuSystem* menu = nullptr;
PlaceholderScreen* miniGamesScreen = nullptr;
CreditsScreen* creditsScreen = nullptr;

// State management
ScreenState currentScreen = WHEEL_SCREEN;
bool menuSelectPressed = false;
bool miniGamesSelectPressed = false;
bool creditsSelectPressed = false;

// Touch wheel variables (temporary until TouchWheel class is complete)
int touch1, touch2, touch3;
int baseline1 = 0, baseline2 = 0, baseline3 = 0;
bool calibrated = false;
int calibrationCount = 0;
float currentAngle = 0;
int ledPosition = -1;
unsigned long lastTouchTime = 0;

// Display state variables
bool needsWheelRedraw = true;
int lastDev1 = -1, lastDev2 = -1, lastDev3 = -1;

// Wheel visualization state
int lastSegmentDrawn = -1;
bool wheelInitialized = false;
int lastPointX = -1;
int lastPointY = -1;
bool wasShowingAngle = false;
int lastSegmentForTouch = -1;
float lastDisplayedAngle = -1;
int lastDisplayedSegment = -1;
bool lastTouchedState = false;
bool selectPressed = false;
unsigned long selectPressTime = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Touch Wheel Badge - Modular");
  
  // Initialize pins
  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);
  pinMode(LED_3_PIN, OUTPUT);
  pinMode(LED_4_PIN, OUTPUT);
  pinMode(LED_5_PIN, OUTPUT);
  pinMode(LED_6_PIN, OUTPUT);
  
  // Turn off all LEDs
  digitalWrite(LED_1_PIN, LOW);
  digitalWrite(LED_2_PIN, LOW);
  digitalWrite(LED_3_PIN, LOW);
  digitalWrite(LED_4_PIN, LOW);
  digitalWrite(LED_5_PIN, LOW);
  digitalWrite(LED_6_PIN, LOW);
  
  // Initialize audio
  Audio::init();
  
  // Initialize display
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  
  // Create objects
  menu = new MenuSystem(&tft);
  miniGamesScreen = new PlaceholderScreen(&tft, "Mini Games");
  creditsScreen = new CreditsScreen(&tft);
  
  // Start with touch wheel demo
  tft.setCursor(10, 10);
  tft.println("Touch Wheel Visualization");
  tft.setCursor(10, 30);
  tft.println("Calibrating... Don't touch");
  calibrated = false;
  calibrationCount = 0;
}

void loop() {
  // Read select button
  int selectTouch = touchRead(SELECT_PIN);
  
  // Debug current screen
  static ScreenState lastScreen = WHEEL_SCREEN;
  if (currentScreen != lastScreen) {
    Serial.print("Screen changed to: ");
    Serial.println(currentScreen);
    lastScreen = currentScreen;
  }
  
  // Handle different screens
  switch (currentScreen) {
    case WHEEL_SCREEN:
      handleWheelScreen(selectTouch);
      break;
      
    case MENU_SCREEN:
      handleMenuScreen(selectTouch);
      break;
      
    case MINI_GAMES_SCREEN:
      handleMiniGamesScreen(selectTouch);
      break;
      
    case CREDITS_SCREEN:
      handleCreditsScreen(selectTouch);
      break;
  }
  
  delay(50);
}

void handleCreditsScreen(int selectTouch) {
  // Handle menu button
  if (selectTouch < TOUCH_THRESHOLD && !creditsSelectPressed) {
    creditsSelectPressed = true;
    Audio::playMenuTone();
    
    // Switch to menu
    currentScreen = MENU_SCREEN;
    tft.fillScreen(TFT_BLACK);
    menu->init();
    menu->draw();
    return;
  } else if (selectTouch >= TOUCH_THRESHOLD) {
    creditsSelectPressed = false;
  }
}

void handleWheelScreen(int selectTouch) {
  // Read touch values
  touch1 = touchRead(Q1_PIN);
  touch2 = touchRead(Q2_PIN);
  touch3 = touchRead(Q3_PIN);
  
  // Handle menu button
  if (selectTouch < TOUCH_THRESHOLD && !menuSelectPressed) {
    menuSelectPressed = true;
    Audio::playMenuTone();
    
    // Switch to menu
    currentScreen = MENU_SCREEN;
    tft.fillScreen(TFT_BLACK);
    menu->init();
    // Pass the calibrated baselines to the menu
    menu->setBaselines(baseline1, baseline2, baseline3);
    menu->draw();
    return;
  } else if (selectTouch >= TOUCH_THRESHOLD) {
    menuSelectPressed = false;
  }
  
  // Handle calibration
  if (!calibrated) {
    if (calibrationCount < CALIBRATION_SAMPLES) {
      if (calibrationCount == 0) {
        baseline1 = touch1;
        baseline2 = touch2;
        baseline3 = touch3;
      } else {
        baseline1 = (baseline1 * 9 + touch1) / 10;
        baseline2 = (baseline2 * 9 + touch2) / 10;
        baseline3 = (baseline3 * 9 + touch3) / 10;
      }
      calibrationCount++;
      
      // Show progress
      int progress = (calibrationCount * 200) / CALIBRATION_SAMPLES;
      tft.fillRect(20, 50, progress, 10, COLOR_SELECT_PRESSED);
      tft.drawRect(20, 50, 200, 10, COLOR_WHEEL_OUTLINE);
      
      return;  // Removed delay(10) - already have delay(50) at end of loop
    } else {
      calibrated = true;
      tft.fillScreen(TFT_BLACK);
      drawWheelInterface();
      needsWheelRedraw = true;
      wheelInitialized = false;  // Force full wheel redraw
    }
  }
  
  // Calculate touch deviations
  int deviation1 = baseline1 - touch1;
  int deviation2 = baseline2 - touch2;
  int deviation3 = baseline3 - touch3;
  
  deviation1 = (deviation1 < 5) ? 0 : deviation1;
  deviation2 = (deviation2 < 5) ? 0 : deviation2;
  deviation3 = (deviation3 < 5) ? 0 : deviation3;
  
  // Update display if changed
  if (deviation1 != lastDev1 || deviation2 != lastDev2 || deviation3 != lastDev3) {
    updateSensorDisplay(deviation1, deviation2, deviation3);
    lastDev1 = deviation1;
    lastDev2 = deviation2;
    lastDev3 = deviation3;
  }
  
  // Handle touch
  bool touched = (deviation1 > 5 || deviation2 > 5 || deviation3 > 5);
  if (touched) {
    lastTouchTime = millis();
    
    // Calculate angle and update LEDs
    float angle = calculateAngle(deviation1, deviation2, deviation3);
    currentAngle = angle;
    
    // Get LED position
    int newLedPosition = angleToLedPosition(angle);
    if (newLedPosition != ledPosition) {
      ledPosition = newLedPosition;
      updateLEDs(ledPosition);
    }
    
    // Calculate visual segment
    int visualSegment = (int)(angle / 60) % 6;
    
    // Update visualization
    if (abs(angle - lastDisplayedAngle) > 2.0 || visualSegment != lastDisplayedSegment || touched != lastTouchedState) {
      drawWheel(angle, true, visualSegment);
      lastDisplayedAngle = angle;
      lastDisplayedSegment = visualSegment;
      lastTouchedState = touched;
    }
  } else {
    if (millis() - lastTouchTime > TOUCH_TIMEOUT) {
      turnOffAllLEDs();
      ledPosition = -1;
      if (lastTouchedState) {
        drawWheel(0, false, -1);
        lastTouchedState = false;
        lastDisplayedSegment = -1;
      }
    }
  }
  
  // Handle select button press in wheel view
  if (selectTouch < TOUCH_THRESHOLD && !selectPressed) {
    selectPressed = true;
    selectPressTime = millis();
  } else if (selectTouch >= TOUCH_THRESHOLD) {
    selectPressed = false;
  }
  
  // Update select button visual
  bool shouldShowPressed = selectPressed && (millis() - selectPressTime < SELECT_VISUAL_DURATION);
  static bool lastSelectVisual = false;
  if (shouldShowPressed != lastSelectVisual) {
    drawCenterButton(shouldShowPressed);
    lastSelectVisual = shouldShowPressed;
  }
}

void handleMenuScreen(int selectTouch) {
  // Read touch for navigation
  touch1 = touchRead(Q1_PIN);
  touch2 = touchRead(Q2_PIN);
  touch3 = touchRead(Q3_PIN);
  
  // Ensure baselines
  if (baseline1 == 0 || baseline2 == 0 || baseline3 == 0) {
    baseline1 = touch1;
    baseline2 = touch2;
    baseline3 = touch3;
  }
  
  // Calculate deviations
  int deviation1 = baseline1 - touch1;
  int deviation2 = baseline2 - touch2;
  int deviation3 = baseline3 - touch3;
  
  deviation1 = (deviation1 < 5) ? 0 : deviation1;
  deviation2 = (deviation2 < 5) ? 0 : deviation2;
  deviation3 = (deviation3 < 5) ? 0 : deviation3;
  
  // Pass raw touch values to menu for better tracking
  menu->handleInput(touch1, touch2, touch3, false);
  
  // Handle selection
  if (selectTouch < TOUCH_THRESHOLD && !menuSelectPressed) {
    menuSelectPressed = true;
    ScreenState newScreen = menu->select();
    currentScreen = newScreen;
    tft.fillScreen(TFT_BLACK);
    
    if (currentScreen == WHEEL_SCREEN) {
      calibrated = false;
      calibrationCount = 0;
      tft.setCursor(10, 10);
      tft.println("Touch Wheel Visualization");
      tft.setCursor(10, 30);
      tft.println("Calibrating... Don't touch");
    } else if (currentScreen == MINI_GAMES_SCREEN) {
      miniGamesScreen->init();
      miniGamesScreen->draw();
    } else if (currentScreen == CREDITS_SCREEN) {
      creditsScreen->init();
      creditsScreen->draw();
    }
  } else if (selectTouch >= TOUCH_THRESHOLD) {
    menuSelectPressed = false;
  }
}

// Temporary wheel interface functions
void drawWheelInterface() {
  tft.setTextSize(2);
  tft.setTextColor(COLOR_TEXT);
  tft.setCursor(30, 10);
  tft.println("Touch Wheel");
  
  tft.setTextSize(1);
  tft.drawRect(10, 200, 220, 70, COLOR_WHEEL_OUTLINE);
  tft.setCursor(15, 205);
  tft.println("Sensor Readings:");
}

void updateSensorDisplay(int dev1, int dev2, int dev3) {
  tft.fillRect(15, 220, 200, 45, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_TEXT);
  
  tft.setCursor(15, 220);
  tft.printf("Q1: %3d ", dev1);
  drawBar(70, 220, dev1);
  
  tft.setCursor(15, 235);
  tft.printf("Q2: %3d ", dev2);
  drawBar(70, 235, dev2);
  
  tft.setCursor(15, 250);
  tft.printf("Q3: %3d ", dev3);
  drawBar(70, 250, dev3);
}

void turnOffAllLEDs() {
  digitalWrite(LED_1_PIN, LOW);
  digitalWrite(LED_2_PIN, LOW);
  digitalWrite(LED_3_PIN, LOW);
  digitalWrite(LED_4_PIN, LOW);
  digitalWrite(LED_5_PIN, LOW);
  digitalWrite(LED_6_PIN, LOW);
}

float calculateAngle(int dev1, int dev2, int dev3) {
  float sum = dev1 + dev2 + dev3;
  if (sum < 5) return currentAngle;
  
  float norm1 = dev1 / sum;
  float norm2 = dev2 / sum;
  float norm3 = dev3 / sum;
  
  float x = norm1 - 0.5f * (norm2 + norm3);
  float y = 0.866f * (norm2 - norm3);
  
  float angle = atan2(y, x) * 180.0f / PI;
  if (angle < 0) angle += 360.0f;
  
  currentAngle = angle;
  return angle;
}

int angleToLedPosition(float angle) {
  int position = (int)(angle / 60) % 6;
  return (6 - position) % 6;  // Reverse for correct direction
}

void updateLEDs(int position) {
  turnOffAllLEDs();
  switch (position) {
    case 0: digitalWrite(LED_1_PIN, HIGH); break;
    case 1: digitalWrite(LED_2_PIN, HIGH); break;
    case 2: digitalWrite(LED_3_PIN, HIGH); break;
    case 3: digitalWrite(LED_4_PIN, HIGH); break;
    case 4: digitalWrite(LED_5_PIN, HIGH); break;
    case 5: digitalWrite(LED_6_PIN, HIGH); break;
  }
}

void drawWheel(float angle, bool touched, int segment) {
  const int centerX = WHEEL_CENTER_X;
  const int centerY = WHEEL_CENTER_Y;
  const int outerRadius = WHEEL_OUTER_RADIUS;
  const int innerRadius = WHEEL_INNER_RADIUS;
  
  // Initial wheel draw
  if (!wheelInitialized || needsWheelRedraw) {
    wheelInitialized = true;
    needsWheelRedraw = false;
    
    tft.fillCircle(centerX, centerY, outerRadius + 5, TFT_BLACK);
    
    // Draw all segments
    for (int i = 0; i < 6; i++) {
      float startAngle = i * 60.0f;
      float endAngle = (i + 1) * 60.0f;
      uint16_t segmentColor = (i == segment && touched) ? COLOR_SEGMENT_ACTIVE : COLOR_WHEEL_BG;
      drawSegment(centerX, centerY, innerRadius, outerRadius, startAngle, endAngle, segmentColor);
    }
    
    // Draw outlines
    tft.drawCircle(centerX, centerY, outerRadius, COLOR_WHEEL_OUTLINE);
    tft.drawCircle(centerX, centerY, innerRadius, COLOR_WHEEL_OUTLINE);
    
    // Draw dividers
    for (int i = 0; i < 6; i++) {
      float lineAngle = i * 60.0f * PI / 180.0f;
      int x1 = centerX - cos(lineAngle) * innerRadius;
      int y1 = centerY + sin(lineAngle) * innerRadius;
      int x2 = centerX - cos(lineAngle) * outerRadius;
      int y2 = centerY + sin(lineAngle) * outerRadius;
      tft.drawLine(x1, y1, x2, y2, COLOR_WHEEL_OUTLINE);
    }
    
    lastSegmentDrawn = segment;
  }
  else if (segment != lastSegmentDrawn) {
    // Redraw changed segments only
    if (lastPointX != -1) {
      tft.fillRect(lastPointX - 13, lastPointY - 13, 26, 26, TFT_BLACK);
    }
    
    if (lastSegmentDrawn >= 0 && lastSegmentDrawn < 6) {
      float startAngle = lastSegmentDrawn * 60.0f;
      float endAngle = (lastSegmentDrawn + 1) * 60.0f;
      drawSegment(centerX, centerY, innerRadius, outerRadius, startAngle, endAngle, COLOR_WHEEL_BG);
    }
    
    if (segment >= 0 && segment < 6 && touched) {
      float startAngle = segment * 60.0f;
      float endAngle = (segment + 1) * 60.0f;
      drawSegment(centerX, centerY, innerRadius, outerRadius, startAngle, endAngle, COLOR_SEGMENT_ACTIVE);
    }
    
    // Redraw affected dividers
    int segments[] = {lastSegmentDrawn, segment};
    for (int j = 0; j < 2; j++) {
      if (segments[j] >= 0 && segments[j] < 6) {
        for (int offset = 0; offset <= 1; offset++) {
          int lineIndex = (segments[j] + offset) % 6;
          float lineAngle = lineIndex * 60.0f * PI / 180.0f;
          int x1 = centerX - cos(lineAngle) * innerRadius;
          int y1 = centerY + sin(lineAngle) * innerRadius;
          int x2 = centerX - cos(lineAngle) * outerRadius;
          int y2 = centerY + sin(lineAngle) * outerRadius;
          tft.drawLine(x1, y1, x2, y2, COLOR_WHEEL_OUTLINE);
        }
      }
    }
    
    tft.drawCircle(centerX, centerY, outerRadius, COLOR_WHEEL_OUTLINE);
    tft.drawCircle(centerX, centerY, innerRadius, COLOR_WHEEL_OUTLINE);
    
    lastSegmentDrawn = segment;
  }
  
  // Draw touch position
  if (touched) {
    float pointAngle = angle * PI / 180.0f;
    int pointRadius = (outerRadius + innerRadius) / 2;
    int pointX = centerX - cos(pointAngle) * pointRadius;
    int pointY = centerY + sin(pointAngle) * pointRadius;
    
    if (segment != lastSegmentForTouch && lastSegmentForTouch != -1) {
      if (lastPointX != -1) {
        tft.fillCircle(lastPointX, lastPointY, 12, TFT_BLACK);
      }
      
      float oldStartAngle = lastSegmentForTouch * 60.0f;
      float oldEndAngle = (lastSegmentForTouch + 1) * 60.0f;
      drawSegment(centerX, centerY, innerRadius, outerRadius, oldStartAngle, oldEndAngle, COLOR_WHEEL_BG);
      
      for (int offset = 0; offset <= 1; offset++) {
        int lineIndex = (lastSegmentForTouch + offset) % 6;
        float lineAngle = lineIndex * 60.0f * PI / 180.0f;
        int x1 = centerX - cos(lineAngle) * innerRadius;
        int y1 = centerY + sin(lineAngle) * innerRadius;
        int x2 = centerX - cos(lineAngle) * outerRadius;
        int y2 = centerY + sin(lineAngle) * outerRadius;
        tft.drawLine(x1, y1, x2, y2, COLOR_WHEEL_OUTLINE);
      }
      
      tft.drawCircle(centerX, centerY, outerRadius, COLOR_WHEEL_OUTLINE);
      tft.drawCircle(centerX, centerY, innerRadius, COLOR_WHEEL_OUTLINE);
    }
    else if (lastPointX != -1 && (abs(pointX - lastPointX) > 2 || abs(pointY - lastPointY) > 2)) {
      tft.fillCircle(lastPointX, lastPointY, 12, TFT_BLACK);
    }
    
    lastSegmentForTouch = segment;
    
    tft.fillCircle(pointX, pointY, 8, COLOR_TOUCH_POINT);
    tft.drawCircle(pointX, pointY, 10, COLOR_TEXT);
    lastPointX = pointX;
    lastPointY = pointY;
    
    static float lastShownAngle = -1;
    if (abs(angle - lastShownAngle) > 2.0) {
      tft.fillRect(80, 170, 80, 20, TFT_BLACK);
      tft.setCursor(85, 175);
      tft.setTextColor(COLOR_TEXT);
      tft.printf("%.0f deg", angle);
      lastShownAngle = angle;
      wasShowingAngle = true;
    }
  } else {
    if (lastPointX != -1) {
      tft.fillCircle(lastPointX, lastPointY, 12, TFT_BLACK);
      
      if (lastSegmentForTouch != -1) {
        float oldStartAngle = lastSegmentForTouch * 60.0f;
        float oldEndAngle = (lastSegmentForTouch + 1) * 60.0f;
        drawSegment(centerX, centerY, innerRadius, outerRadius, oldStartAngle, oldEndAngle, COLOR_WHEEL_BG);
        
        for (int offset = 0; offset <= 1; offset++) {
          int lineIndex = (lastSegmentForTouch + offset) % 6;
          float lineAngle = lineIndex * 60.0f * PI / 180.0f;
          int x1 = centerX - cos(lineAngle) * innerRadius;
          int y1 = centerY + sin(lineAngle) * innerRadius;
          int x2 = centerX - cos(lineAngle) * outerRadius;
          int y2 = centerY + sin(lineAngle) * outerRadius;
          tft.drawLine(x1, y1, x2, y2, COLOR_WHEEL_OUTLINE);
        }
        
        lastSegmentForTouch = -1;
      }
      
      lastPointX = -1;
      lastPointY = -1;
    }
    
    if (wasShowingAngle) {
      tft.fillRect(80, 170, 80, 20, TFT_BLACK);
      wasShowingAngle = false;
    }
  }
  
  static bool lastButtonState = false;
  if (selectPressed != lastButtonState || needsWheelRedraw) {
    drawCenterButton(selectPressed);
    lastButtonState = selectPressed;
  }
}

void drawSegment(int cx, int cy, int innerR, int outerR, float startAngle, float endAngle, uint16_t color) {
  float start = startAngle * PI / 180.0f;
  float end = endAngle * PI / 180.0f;
  float mid = (start + end) / 2.0f;
  
  int x1 = cx - cos(start) * innerR;
  int y1 = cy + sin(start) * innerR;
  int x2 = cx - cos(start) * outerR;
  int y2 = cy + sin(start) * outerR;
  int x3 = cx - cos(mid) * outerR;
  int y3 = cy + sin(mid) * outerR;
  int x4 = cx - cos(end) * outerR;
  int y4 = cy + sin(end) * outerR;
  int x5 = cx - cos(end) * innerR;
  int y5 = cy + sin(end) * innerR;
  int x6 = cx - cos(mid) * innerR;
  int y6 = cy + sin(mid) * innerR;
  
  tft.fillTriangle(x1, y1, x2, y2, x3, y3, color);
  tft.fillTriangle(x1, y1, x3, y3, x6, y6, color);
  tft.fillTriangle(x6, y6, x3, y3, x4, y4, color);
  tft.fillTriangle(x6, y6, x4, y4, x5, y5, color);
}

void drawCenterButton(bool pressed) {
  const int centerX = WHEEL_CENTER_X;
  const int centerY = WHEEL_CENTER_Y;
  const int buttonRadius = 20;
  
  uint16_t buttonColor = pressed ? COLOR_SELECT_PRESSED : COLOR_WHEEL_OUTLINE;
  uint16_t fillColor = pressed ? COLOR_SELECT_PRESSED : COLOR_WHEEL_BG;
  
  tft.fillCircle(centerX, centerY, buttonRadius, fillColor);
  tft.drawCircle(centerX, centerY, buttonRadius, buttonColor);
  
  tft.setCursor(centerX - 10, centerY - 4);
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(1);
  tft.print("SEL");
}

void drawBar(int x, int y, int value) {
  int barLength = constrain(value * 2, 0, 140);
  if (barLength > 0) {
    tft.fillRect(x, y, barLength, 8, COLOR_SEGMENT_ACTIVE);
  }
  tft.drawRect(x, y, 140, 8, COLOR_WHEEL_OUTLINE);
}

// Removed functions for WiFi, Comms, and Game screens

void handleMiniGamesScreen(int selectTouch) {
  // Handle menu button
  if (selectTouch < TOUCH_THRESHOLD && !miniGamesSelectPressed) {
    miniGamesSelectPressed = true;
    Audio::playMenuTone();
    
    // Switch to menu
    currentScreen = MENU_SCREEN;
    tft.fillScreen(TFT_BLACK);
    menu->init();
    menu->draw();
    return;
  } else if (selectTouch >= TOUCH_THRESHOLD) {
    miniGamesSelectPressed = false;
  }
}