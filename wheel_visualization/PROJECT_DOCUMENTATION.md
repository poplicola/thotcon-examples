# ESP32 Touch Wheel Badge Documentation

## Project Overview
This project implements an interactive touch wheel interface for an ESP32-based badge with a TFT display. The badge features:
- A circular capacitive touch wheel with 6 LEDs
- A center select button
- A TFT color display (240x320 pixels)
- A buzzer for audio feedback

## Current Implementation Status

### Completed Features
1. **Splash Screen with Sprite Display**
   - Displays a 32x32 pixel crown sprite on startup
   - Plays Close Encounters theme (D4-E4-C4-C3-G3) on first boot only
   - Shows "Press SELECT to continue" message

2. **Touch Wheel Visualization**
   - Real-time finger position tracking with red dot indicator
   - Orange segment highlighting for active touch region
   - Angle display showing touch position in degrees
   - Visual feedback synchronized with physical LEDs
   - Smooth trailing effect for touch movement

3. **Menu System**
   - Accessed by pressing SELECT on the wheel screen
   - Two menu items: "Main" (returns to splash) and "Touch Wheel"
   - Navigation using touch wheel (top half = up, bottom half = down)
   - Visual highlighting of selected menu item

4. **Physical Hardware Integration**
   - 6 LEDs that light up based on touch position
   - LEDs move clockwise when finger moves clockwise
   - Center button detection with audio feedback
   - Capacitive touch sensing on 3 pins (Q1, Q2, Q3)

### Known Issues Resolved
- Screen flickering during updates (fixed with selective redrawing)
- Touch position mirroring (fixed with coordinate corrections)
- LED direction reversed from finger movement (fixed with position reversal)
- Segment highlighting misaligned with touch (fixed by separating visual/LED logic)
- Trailing artifacts from touch indicator (fixed with larger clearing area)

## TFT Drawing Implementation Guide

### Display Hardware
- **Display Type**: TFT_eSPI library compatible display
- **Resolution**: 240x320 pixels
- **Color Format**: 16-bit RGB565

### Key Drawing Principles

#### 1. Minimize Screen Updates
```cpp
// BAD: Redrawing entire screen frequently
void updateDisplay() {
  tft.fillScreen(TFT_BLACK);  // Causes flicker!
  drawEverything();
}

// GOOD: Only update what changed
static int lastValue = -1;
if (value != lastValue) {
  tft.fillRect(x, y, width, height, TFT_BLACK);  // Clear old
  drawNewValue(value);
  lastValue = value;
}
```

#### 2. Color Definitions (RGB565 format)
```cpp
#define COLOR_WHEEL_BG 0x1082       // Dark blue background
#define COLOR_WHEEL_OUTLINE 0x4208  // Medium gray
#define COLOR_TOUCH_POINT 0xF800    // Red
#define COLOR_SELECT_PRESSED 0x07E0 // Green
#define COLOR_SEGMENT_ACTIVE 0xFD20 // Orange
#define COLOR_TEXT 0xFFFF           // White
```

#### 3. Drawing Primitives Used
```cpp
// Basic shapes
tft.fillCircle(x, y, radius, color);     // Filled circle
tft.drawCircle(x, y, radius, color);     // Circle outline
tft.fillRect(x, y, width, height, color); // Filled rectangle
tft.drawLine(x1, y1, x2, y2, color);     // Line
tft.fillTriangle(x1,y1,x2,y2,x3,y3,color); // Filled triangle

// Text
tft.setTextSize(2);              // 2x normal size
tft.setTextColor(COLOR_TEXT);    // Text color
tft.setCursor(x, y);             // Position
tft.print("Text");               // Print text
```

#### 4. Sprite Drawing (1-bit monochrome)
```cpp
// Sprites stored as byte arrays in PROGMEM
const PROGMEM uint8_t sprite[] = { /* data */ };

// Drawing pixel by pixel
for (int y = 0; y < height; y++) {
  for (int x = 0; x < width; x++) {
    int byteIndex = (y * width + x) / 8;
    int bitIndex = 7 - ((y * width + x) % 8);
    uint8_t pixelByte = pgm_read_byte(&sprite[byteIndex]);
    bool pixelOn = (pixelByte >> bitIndex) & 0x01;
    if (pixelOn) {
      tft.drawPixel(x + offsetX, y + offsetY, color);
    }
  }
}
```

#### 5. Flicker Prevention Strategies

**A. Track State Changes**
```cpp
static bool lastState = false;
if (currentState != lastState) {
  // Only redraw when state changes
  redrawElement();
  lastState = currentState;
}
```

**B. Selective Area Clearing**
```cpp
// Clear only the area that will change
tft.fillCircle(oldX, oldY, radius + margin, TFT_BLACK);
// Draw new content
tft.fillCircle(newX, newY, radius, color);
```

**C. Double Buffer Key Areas**
```cpp
// For the wheel segments, we track what's drawn
static int lastSegmentDrawn = -1;
if (segment != lastSegmentDrawn) {
  // Only redraw the two affected segments
  drawSegment(lastSegmentDrawn, COLOR_WHEEL_BG);
  drawSegment(segment, COLOR_SEGMENT_ACTIVE);
  lastSegmentDrawn = segment;
}
```

### Touch Wheel Drawing Architecture

#### Coordinate System Fixes
The touch wheel had mirroring issues that required coordinate corrections:
```cpp
// Original (mirrored horizontally and vertically)
int pointX = centerX + cos(angle) * radius;
int pointY = centerY + sin(angle) * radius;

// Fixed (correct orientation)
int pointX = centerX - cos(angle) * radius;  // Negated X
int pointY = centerY + sin(angle) * radius;  // Positive Y
```

#### Segment Drawing
Segments are drawn as composite shapes using triangles:
```cpp
void drawSegment(int cx, int cy, int innerR, int outerR, 
                 float startAngle, float endAngle, uint16_t color) {
  // Convert to radians and calculate points
  // Draw using multiple triangles to approximate arc
  // This creates smooth-looking segments
}
```

#### Touch Indicator Trailing Fix
The touch indicator left artifacts due to incomplete clearing:
```cpp
// Problem: Only clearing the filled circle
tft.fillCircle(x, y, 8, TFT_BLACK);

// Solution: Clear larger area including outline
tft.fillCircle(x, y, 12, TFT_BLACK);  // Covers 10px outline
// OR for segment changes:
tft.fillRect(x-13, y-13, 26, 26, TFT_BLACK);  // Rectangle clear
```

## Screen State Management

### Three Screen States
```cpp
enum ScreenState {
  SPLASH_SCREEN,   // Crown sprite + intro
  WHEEL_SCREEN,    // Touch wheel interface  
  MENU_SCREEN      // Navigation menu
};
```

### State Transition Flow
1. **Startup** → SPLASH_SCREEN
2. **SPLASH + SELECT** → WHEEL_SCREEN  
3. **WHEEL + SELECT** → MENU_SCREEN
4. **MENU + "Main"** → SPLASH_SCREEN
5. **MENU + "Touch Wheel"** → WHEEL_SCREEN

### Important State Management Rules
- Each screen handles only its own input
- Clear screen on transitions
- Reset state flags when entering new screen
- Use separate button press flags per screen to avoid conflicts

## Touch Sensing Implementation

### Hardware Configuration
- **Q1_PIN (13)**: First capacitive sensor
- **Q2_PIN (12)**: Second capacitive sensor  
- **Q3_PIN (14)**: Third capacitive sensor
- **SELECT_PIN (27)**: Center button sensor

### Angle Calculation
```cpp
// Three sensors at 0°, 120°, and 240°
float calculateAngle(int dev1, int dev2, int dev3) {
  float sum = dev1 + dev2 + dev3;
  float norm1 = dev1 / sum;  // Normalize
  float norm2 = dev2 / sum;
  float norm3 = dev3 / sum;
  
  // Triangulation using sensor positions
  float x = norm1 - 0.5f * (norm2 + norm3);
  float y = 0.866f * (norm2 - norm3);
  
  float angle = atan2(y, x) * 180.0f / PI;
  if (angle < 0) angle += 360.0f;
  return angle;
}
```

### LED Mapping
LEDs are reversed to match clockwise movement:
```cpp
int angleToLedPosition(float angle) {
  int position = (int)(angle / 60) % 6;
  return (6 - position) % 6;  // Reverse for correct direction
}
```

## Future Improvements Needed

1. **Menu System Enhancements**
   - Add more menu items for additional features
   - Implement scrolling for longer menus
   - Add icons or visual indicators

2. **Touch Wheel Features**
   - Add gesture recognition (swipes, taps)
   - Implement variable sensitivity settings
   - Add haptic feedback if hardware supports it

3. **Visual Enhancements**
   - Smooth animation transitions
   - More sprite animations
   - Color themes or customization

4. **Performance Optimizations**
   - Further reduce drawing operations
   - Implement dirty rectangle tracking
   - Consider partial screen refresh if supported

## Critical Implementation Notes

1. **Always test on actual hardware** - The TFT behavior can differ from expectations
2. **Watch for memory usage** - ESP32 has limited RAM, sprites and buffers add up
3. **Calibration is essential** - Touch sensors need baseline calibration on startup
4. **Timing matters** - Some displays need delays between operations
5. **Coordinate systems** - Always verify orientation with test patterns

## Debugging Tips

1. Use Serial output liberally during development
2. Draw test patterns to verify coordinate system
3. Add visual indicators for touch regions during testing
4. Log state transitions to catch logic errors
5. Use distinct colors during development, refine later

This documentation will be updated as the project evolves.

## Project Structure (Refactored)

The project has been reorganized into modular components:

### Header Files
1. **config.h** - All pin definitions, constants, and color definitions
2. **sprites.h/cpp** - Sprite data and drawing functions
3. **menu.h/cpp** - Menu system class with navigation logic
4. **touch_wheel.h/cpp** - Touch wheel visualization and sensing
5. **audio.h/cpp** - Audio feedback utilities
6. **splash_screen.h/cpp** - Splash screen display logic

### Main Sketch Structure
```cpp
// wheel_visualization.ino
#include "config.h"
#include "menu.h"
#include "touch_wheel.h"
#include "splash_screen.h"
#include "audio.h"

// Global objects
TFT_eSPI tft = TFT_eSPI();
MenuSystem menu(&tft);
TouchWheel wheel(&tft);
SplashScreen splash(&tft);

// State management
ScreenState currentScreen = SPLASH_SCREEN;

void setup() {
  // Initialize hardware
  initializePins();
  Audio::init();
  tft.init();
  // etc...
}

void loop() {
  // Read inputs
  int selectTouch = touchRead(SELECT_PIN);
  
  // Handle current screen
  switch(currentScreen) {
    case SPLASH_SCREEN:
      handleSplashScreen();
      break;
    case WHEEL_SCREEN:
      handleWheelScreen();
      break;
    case MENU_SCREEN:
      handleMenuScreen();
      break;
  }
}
```

### Benefits of Modular Structure
1. **Maintainability** - Each component is self-contained
2. **Reusability** - Components can be used in other projects
3. **Testing** - Individual modules can be tested separately
4. **Collaboration** - Multiple developers can work on different modules
5. **Memory Management** - Only include what you need

### Adding New Features
To add a new screen or feature:
1. Create a new header/cpp pair
2. Define the class interface in the header
3. Implement functionality in the cpp file
4. Include in main sketch and add to screen state handling

### Compilation Notes
- All .cpp files must be in the same directory as the .ino file
- The Arduino IDE will automatically compile all .cpp files
- Use forward declarations to avoid circular dependencies

## Current Project Status (Post-Refactoring)

### What's Been Modularized
1. **config.h** - Centralized configuration
   - All pin definitions (LEDs, touch sensors, buzzer)
   - Timing constants (timeouts, delays)
   - Display dimensions and positions
   - Color scheme definitions

2. **sprites.h/cpp** - Sprite management
   - Sprite data storage (troopers_crown)
   - Generic drawSprite() function for 1-bit monochrome sprites
   - Easy to add new sprites

3. **menu.h/cpp** - Complete menu system
   - MenuSystem class with navigation logic
   - Handles touch wheel input for up/down navigation
   - Menu item selection with audio feedback
   - Easily extensible with new menu items

4. **audio.h/cpp** - Audio utilities
   - Static class for all sound effects
   - Close Encounters intro (plays once)
   - Menu navigation tones
   - Selection confirmation sounds

5. **splash_screen.h/cpp** - Splash screen
   - Displays crown sprite
   - Shows "Press SELECT to continue"
   - Triggers intro music playback

### What Remains in Main Sketch
The touch wheel visualization code currently remains in the main sketch (wheel_visualization.ino) including:
- Wheel drawing functions (drawWheel, drawSegment)
- Touch position tracking
- Segment highlighting logic
- Sensor display with bars
- LED control
- Angle calculations

### Recent Fixes Applied
1. **Calibration Speed** - Removed redundant delay(10) during calibration, making it 2x faster
2. **Full Wheel Visualization** - Restored all drawing functions that were temporarily removed
3. **State Management** - Proper separation between screen states prevents overlap

## Next Steps for Full Modularization

### Create TouchWheel Class
The logical next step is to extract all touch wheel functionality into its own class:

```cpp
// touch_wheel.h
class TouchWheel {
private:
  TFT_eSPI* tft;
  
  // Calibration state
  int baseline1, baseline2, baseline3;
  bool calibrated;
  int calibrationCount;
  
  // Touch tracking
  float currentAngle;
  int currentSegment;
  int ledPosition;
  
  // Display state
  int lastSegmentDrawn;
  bool wheelInitialized;
  int lastPointX, lastPointY;
  // ... other state variables
  
  // Private drawing methods
  void drawSegment(int cx, int cy, int innerR, int outerR, 
                   float startAngle, float endAngle, uint16_t color);
  void drawWheel(float angle, bool touched, int segment);
  void updateLEDs(int position);
  
public:
  TouchWheel(TFT_eSPI* display);
  
  // Main interface
  void init();
  void calibrate(int touch1, int touch2, int touch3);
  bool isCalibrated() { return calibrated; }
  void update(int touch1, int touch2, int touch3);
  
  // State getters
  float getAngle() { return currentAngle; }
  int getSegment() { return currentSegment; }
  bool isTouched() { return /* touch logic */; }
  
  // Display methods
  void draw();
  void drawInterface();
  void drawSensorReadings(int dev1, int dev2, int dev3);
  void drawCenterButton(bool pressed);
};
```

### Benefits of TouchWheel Class
1. **Encapsulation** - All wheel-related state in one place
2. **Reusability** - Can be used in other projects
3. **Cleaner Main Loop** - Just call wheel.update() and wheel.draw()
4. **Easier Testing** - Can test wheel functionality in isolation
5. **Better State Management** - No global variables for wheel state

### Implementation Plan
1. Create touch_wheel.h with class definition
2. Move all wheel-related variables to private members
3. Move all wheel functions to class methods
4. Update main sketch to use TouchWheel instance
5. Test thoroughly to ensure no functionality is lost

### Main Sketch After Full Modularization
```cpp
void handleWheelScreen(int selectTouch) {
  // Much cleaner!
  touch1 = touchRead(Q1_PIN);
  touch2 = touchRead(Q2_PIN);
  touch3 = touchRead(Q3_PIN);
  
  if (!wheel.isCalibrated()) {
    wheel.calibrate(touch1, touch2, touch3);
    return;
  }
  
  wheel.update(touch1, touch2, touch3);
  
  if (selectTouch < TOUCH_THRESHOLD && !menuSelectPressed) {
    // Open menu
    currentScreen = MENU_SCREEN;
    // ...
  }
}
```

## Known Issues and Considerations

### Memory Usage
- Each class instance uses RAM
- PROGMEM used for sprite data to save RAM
- Monitor free heap space when adding features

### Display Performance
- Minimize full screen clears
- Use selective redrawing (already implemented)
- Consider display buffer if flickering persists

### Touch Sensitivity
- Calibration values may vary between devices
- Consider adding sensitivity adjustment to menu
- Store calibration in EEPROM for persistence

### Future Enhancements
1. **Settings Menu** - Adjust touch sensitivity, colors, sounds
2. **Multiple Sprites** - Animation system for sprites
3. **Games/Apps** - Use the wheel as a game controller
4. **Bluetooth** - Connect to phone for notifications
5. **Battery Monitor** - Show battery level on display

## Touch Wheel Menu Navigation - Implementation Guide

### The Problem: Erratic Menu Behavior
Initial implementations suffered from:
- Cursor jumping when holding finger still
- Unpredictable navigation direction
- Over-sensitive response to minor movements
- Different behavior between screens despite same hardware

### The Solution: Delta-Based Tracking

#### Key Principles for iPod-Like Behavior

1. **Track Changes, Not Absolute Position**
```cpp
// BAD: Using absolute angle
if (angle < 180) navigateUp();
else navigateDown();

// GOOD: Track angle changes
float deltaAngle = angle - lastAngle;
if (deltaAngle > threshold) navigate();
```

2. **Share Calibration Data**
- The touch wheel screen already has perfectly calibrated baseline values
- Pass these to the menu instead of recalibrating
- Consistent behavior across all screens

3. **Accumulate Rotation**
```cpp
// Require 30° of rotation per menu item
angleAccumulator += deltaAngle;
if (angleAccumulator >= 30.0) {
    navigateUp();
    angleAccumulator -= 30.0;
}
```

4. **Filter Noise and Jumps**
```cpp
// Ignore tiny movements (noise)
if (abs(deltaAngle) < 3.0) return;

// Ignore huge jumps (finger repositioning)  
if (abs(deltaAngle) > 90.0) return;
```

5. **Handle Angle Wrapping**
```cpp
// Crossing 0°/360° boundary
if (deltaAngle > 180) deltaAngle -= 360;
if (deltaAngle < -180) deltaAngle += 360;
```

### Current Implementation
```cpp
void MenuSystem::handleInput(int touch1, int touch2, int touch3, bool selectButton) {
    // 1. Use shared baselines from main calibration
    int dev1 = menuBaseline1 - touch1;
    int dev2 = menuBaseline2 - touch2;
    int dev3 = menuBaseline3 - touch3;
    
    // 2. Calculate angle only when touched with sufficient pressure
    if (sum > 10) { // Higher threshold for stability
        float angle = calculateAngle(dev1, dev2, dev3);
        
        // 3. Track changes from last position
        if (wasTracking) {
            float deltaAngle = angle - lastAngle;
            // Handle wrapping...
            
            // 4. Accumulate rotation with noise filtering
            if (abs(deltaAngle) > 3.0 && abs(deltaAngle) < 90.0) {
                angleAccumulator += deltaAngle;
                
                // 5. Navigate at 30° intervals
                if (angleAccumulator >= 30.0) {
                    navigateUp();  // Clockwise = up
                    angleAccumulator -= 30.0;
                }
            }
        }
    }
}
```

### Why This Works
- **No jumping**: Only responds to deliberate rotation, not static touch
- **Predictable**: Always requires same amount of rotation (30°) per item
- **Natural feel**: Matches muscle memory from iPod/rotary encoders
- **Consistent**: Same calibration used everywhere
- **Robust**: Filters out noise and finger repositioning

### Design Decisions
- **30° per item**: Comfortable rotation distance, not too sensitive
- **3° dead zone**: Eliminates jitter when holding still
- **90° jump filter**: Prevents glitches from finger repositioning
- **10+ total deviation**: Ensures solid finger contact before tracking
- **Clockwise = Up**: Matches most rotary control conventions

### Key Learnings
1. **Never track absolute position for menus** - Always use relative changes
2. **Share calibration globally** - Don't recalibrate for each screen
3. **Filter aggressively** - Better to miss a tiny movement than jump erratically  
4. **Reset on release** - Clean state prevents accumulated errors
5. **Test with real fingers** - Synthetic tests miss real-world usage patterns

This approach creates the "rubbery-smooth" iPod feel users expect, where the menu responds predictably to deliberate rotation while ignoring unintentional touches or position holds.