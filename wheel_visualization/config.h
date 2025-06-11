#ifndef CONFIG_H
#define CONFIG_H

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

// Timing constants
#define TOUCH_TIMEOUT 500  // LED timeout in ms (0.5 seconds)
#define CALIBRATION_SAMPLES 100
#define MENU_NAV_DELAY 300  // Debounce for menu navigation
#define SELECT_VISUAL_DURATION 200

// Display constants
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define WHEEL_CENTER_X 120
#define WHEEL_CENTER_Y 100
#define WHEEL_OUTER_RADIUS 60
#define WHEEL_INNER_RADIUS 25

// Colors for visualization (RGB565 format)
#define COLOR_WHEEL_BG 0x1082       // Dark blue
#define COLOR_WHEEL_OUTLINE 0x4208  // Medium gray
#define COLOR_TOUCH_POINT 0xF800    // Red
#define COLOR_SELECT_PRESSED 0x07E0 // Green
#define COLOR_SEGMENT_ACTIVE 0xFD20 // Orange
#define COLOR_TEXT 0xFFFF           // White

// WiFi Configuration - REMOVED FOR PUBLIC RELEASE
// #define WIFI_SSID "YOUR_WIFI_SSID"
// #define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
// #define WIFI_TIMEOUT_MS 10000  // 10 seconds

#endif // CONFIG_H