#include "Arduino.h"
#include <TFT_eSPI.h>

// Pin definitions
#define BUZZER_PIN 5

#define LED_1_PIN 21
#define LED_2_PIN 22
#define LED_3_PIN 19
#define LED_4_PIN 17
#define LED_5_PIN 16
#define LED_6_PIN 25

#define SELECT_PIN 27
#define Q1_PIN 13
#define Q2_PIN 12
#define Q3_PIN 14

#define TOUCH_THRESHOLD 30

TFT_eSPI tft = TFT_eSPI();

// Variables for touch processing
int touch1, touch2, touch3;
int baseline1, baseline2, baseline3;
bool calibrated = false;
int calibrationCount = 0;
const int CALIBRATION_SAMPLES = 100;

// Variables for LED display
int currentLed = -1;
unsigned long lastTouchTime = 0;
const unsigned long TOUCH_TIMEOUT = 2000; // LED timeout in ms

// For polar angle calculation
float lastAngle = 0;
float currentAngle = 0;
int ledPosition = -1;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Capacitive Touch Wheel Test");
  
  // Set up LED pins
  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);
  pinMode(LED_3_PIN, OUTPUT);
  pinMode(LED_4_PIN, OUTPUT);
  pinMode(LED_5_PIN, OUTPUT);
  pinMode(LED_6_PIN, OUTPUT);
  
  // Turn off all LEDs initially
  digitalWrite(LED_1_PIN, LOW);
  digitalWrite(LED_2_PIN, LOW);
  digitalWrite(LED_3_PIN, LOW);
  digitalWrite(LED_4_PIN, LOW);
  digitalWrite(LED_5_PIN, LOW);
  digitalWrite(LED_6_PIN, LOW);
  
  // Set up buzzer
  ledcAttach(BUZZER_PIN, 1000, 8);
  
  // Initialize display
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  
  // Display initial message
  tft.setCursor(10, 10);
  tft.println("Capacitive Touch Wheel Test");
  tft.setCursor(10, 30);
  tft.println("Calibrating... Don't touch wheel");
}

void loop() {
  // Read touch values
  touch1 = touchRead(Q1_PIN);
  touch2 = touchRead(Q2_PIN);
  touch3 = touchRead(Q3_PIN);
  int selectTouch = touchRead(SELECT_PIN);
  
  // If select is pressed, beep
  if (selectTouch < TOUCH_THRESHOLD) {
    ledcWriteTone(BUZZER_PIN, 440);
    delay(50);
    ledcWriteTone(BUZZER_PIN, 0);
  }

  // Perform initial calibration
  if (!calibrated) {
    if (calibrationCount < CALIBRATION_SAMPLES) {
      if (calibrationCount == 0) {
        baseline1 = touch1;
        baseline2 = touch2;
        baseline3 = touch3;
      } else {
        // Smooth baseline calculation
        baseline1 = (baseline1 * 9 + touch1) / 10;
        baseline2 = (baseline2 * 9 + touch2) / 10;
        baseline3 = (baseline3 * 9 + touch3) / 10;
      }
      calibrationCount++;
      
      // Display progress
      tft.setCursor(10, 40);
      tft.printf("Calibrating: %d%%   ", (calibrationCount * 100) / CALIBRATION_SAMPLES);
      
      delay(10);
      return;
    } else {
      calibrated = true;
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(10, 10);
      tft.println("Touch Wheel Test");
      tft.setCursor(10, 25);
      tft.println("Baselines:");
      tft.setCursor(10, 35);
      tft.printf("Q1: %d", baseline1);
      tft.setCursor(10, 45);
      tft.printf("Q2: %d", baseline2);
      tft.setCursor(10, 55);
      tft.printf("Q3: %d", baseline3);
    }
  }

  // Calculate deviations from baseline
  int deviation1 = baseline1 - touch1;
  int deviation2 = baseline2 - touch2;
  int deviation3 = baseline3 - touch3;
  
  // Ignore small deviations
  deviation1 = (deviation1 < 5) ? 0 : deviation1;
  deviation2 = (deviation2 < 5) ? 0 : deviation2;
  deviation3 = (deviation3 < 5) ? 0 : deviation3;
  
  // Display raw values and deviations
  tft.setCursor(10, 75);
  tft.printf("Raw Values:               ");
  tft.setCursor(10, 85);
  tft.printf("Q1: %d (dev: %d)   ", touch1, deviation1);
  tft.setCursor(10, 95);
  tft.printf("Q2: %d (dev: %d)   ", touch2, deviation2);
  tft.setCursor(10, 105);
  tft.printf("Q3: %d (dev: %d)   ", touch3, deviation3);
  
  // Detect if wheel is being touched
  bool touched = (deviation1 > 5 || deviation2 > 5 || deviation3 > 5);
  
  if (touched) {
    lastTouchTime = millis();
    
    // Convert the three sensor values to a polar angle
    // Formula based on Bryan Duxbury's method for triangular interpolation
    float angle = calculateAngle(deviation1, deviation2, deviation3);
    currentAngle = angle;
    
    // Get LED position from angle (0-5)
    int newLedPosition = angleToLedPosition(angle);
    
    if (newLedPosition != ledPosition) {
      ledPosition = newLedPosition;
      updateLEDs(ledPosition);
      
      tft.setCursor(10, 130);
      tft.printf("Angle: %.1f degrees   ", angle);
      tft.setCursor(10, 140);
      tft.printf("LED Position: %d   ", ledPosition);
    }
    
    // Debug visualization
    drawTouchPoint(angle);
  } else {
    // If no touch for TOUCH_TIMEOUT ms, turn off LEDs
    if (millis() - lastTouchTime > TOUCH_TIMEOUT) {
      turnOffAllLEDs();
      ledPosition = -1;
    }
  }
  
  // Send data to Serial for external plotting/analysis
  Serial.print(touch1);
  Serial.print(",");
  Serial.print(touch2);
  Serial.print(",");
  Serial.print(touch3);
  Serial.print(",");
  Serial.print(deviation1);
  Serial.print(",");
  Serial.print(deviation2);
  Serial.print(",");
  Serial.print(deviation3);
  Serial.print(",");
  Serial.println(currentAngle);
  
  delay(50); // Update rate
}

// Calculate angle from touch values (0-360 degrees)
float calculateAngle(int dev1, int dev2, int dev3) {
  // Ensure we have some touch input
  float sum = dev1 + dev2 + dev3;
  if (sum < 5) return lastAngle; // Keep last angle if no significant touch
  
  // Normalize values (0.0 to 1.0)
  float norm1 = dev1 / sum;
  float norm2 = dev2 / sum;
  float norm3 = dev3 / sum;
  
  // Calculate angle using trigonometry
  // The three sensors are placed at 0°, 120°, and 240°
  float x = norm1 - 0.5f * (norm2 + norm3);
  float y = 0.866f * (norm2 - norm3); // 0.866 is sin(120°)
  
  float angle = atan2(y, x) * 180.0f / PI;
  if (angle < 0) angle += 360.0f;
  
  // Apply some smoothing
  lastAngle = angle;
  return angle;
}

// Convert angle to LED position (0-5)
int angleToLedPosition(float angle) {
  // Map 360 degrees to 6 positions (each 60 degrees)
  // Add 30 for proper sector alignment, then divide by 60 to get position
  int position = (int)((angle + 30) / 60) % 6;
  
  // Flip the LED position (0→5, 1→4, 2→3, 3→2, 4→1, 5→0)
  return 5 - position;
}

// Update LEDs based on position
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

// Turn off all LEDs
void turnOffAllLEDs() {
  digitalWrite(LED_1_PIN, LOW);
  digitalWrite(LED_2_PIN, LOW);
  digitalWrite(LED_3_PIN, LOW);
  digitalWrite(LED_4_PIN, LOW);
  digitalWrite(LED_5_PIN, LOW);
  digitalWrite(LED_6_PIN, LOW);
}

// Draw a visualization of the touch point on the display
void drawTouchPoint(float angle) {
  const int centerX = 180;
  const int centerY = 150;
  const int radius = 50;
  
  // Clear previous wheel visualization
  tft.fillCircle(centerX, centerY, radius + 5, TFT_BLACK);
  
  // Draw wheel outline
  tft.drawCircle(centerX, centerY, radius, TFT_DARKGREY);
  
  // Draw sector lines
  for (int i = 0; i < 6; i++) {
    float lineAngle = i * 60.0f * PI / 180.0f;
    // Flip horizontally by inverting the X coordinate calculation
    int x2 = centerX - cos(lineAngle) * radius;
    int y2 = centerY + sin(lineAngle) * radius;
    tft.drawLine(centerX, centerY, x2, y2, TFT_DARKGREY);
  }
  
  // Draw current position
  float pointAngle = angle * PI / 180.0f;
  // Flip horizontally by inverting the X coordinate calculation
  int pointX = centerX - cos(pointAngle) * (radius - 10);
  int pointY = centerY + sin(pointAngle) * (radius - 10);
  tft.fillCircle(pointX, pointY, 5, TFT_RED);
}