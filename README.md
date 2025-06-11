# Touch Wheel Badge - Demo Firmware Collection

This repository contains cleaned-up demo firmware for the touch wheel badge, suitable for public release and custom projects. Three different firmware examples are provided to showcase various capabilities of the hardware.

## Repository Contents

### 1. `tonechaser/` - Musical Tone Explorer
Advanced musical demonstration with multiple modes:
- **Free Play Mode**: Continuous tone generation based on touch position
- **Scale Mode**: Quantized musical scales
- **Chord Mode**: Play musical chords
- **Arpeggio Mode**: Automatic arpeggios
- **Visual Wheel**: Colorful wheel visualization with effects
- **Musical Features**: Multiple scales (Major, Minor, Pentatonic, Blues)

### 2. `working/` - Basic Touch Wheel Test
Simple touch wheel demonstration:
- **Touch Detection**: Basic capacitive touch wheel functionality
- **LED Indicators**: 6 LEDs that light up based on touch position
- **Calibration**: Automatic baseline calibration
- **Serial Output**: Debug data for development
- **Visual Feedback**: Simple wheel visualization on display

### 3. `wheel_visualization/` - Full Interactive Demo
Additional badge firmware with menu system:
- **Touch Wheel Demo**: Interactive capacitive touch wheel with visual feedback
- **Menu System**: Navigate between different modes using the touch wheel
- **Audio Feedback**: Simple tone feedback for interactions
- **Mini Games**: Placeholder for future game implementations
- **Credits Screen**: Information about the original creators
- **Clean Architecture**: Modular code structure for easy customization

## Hardware Requirements

All firmware examples work with the same hardware:

- **ESP32 microcontroller** (ESP32 Thing or compatible)
- **ST7789 TFT Display** (240x320 pixels)
- **6 LEDs** connected to GPIO pins
- **3 Capacitive touch sensors** for wheel input
- **Buzzer** for audio feedback
- **Center button** (capacitive touch)

## Pin Configuration

### TFT Display Pins
```cpp
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST   4  // Reset pin
```

### LED Pins
```cpp
#define LED_1_PIN 21
#define LED_2_PIN 22
#define LED_3_PIN 19
#define LED_4_PIN 17
#define LED_5_PIN 16
#define LED_6_PIN 25
```

### Touch Sensor Pins
```cpp
#define SELECT_PIN 27  // Center button
#define Q1_PIN 13      // Touch sensor 1
#define Q2_PIN 12      // Touch sensor 2
#define Q3_PIN 14      // Touch sensor 3
```

### Audio Pin
```cpp
#define BUZZER_PIN 5
```

## Getting Started

### 1. Install Arduino IDE
Download and install the [Arduino IDE](https://www.arduino.cc/en/software) if you haven't already.

### 2. Install ESP32 Board Package
1. In Arduino IDE, go to **File > Preferences**
2. Add this URL to "Additional Board Manager URLs":
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
3. Go to **Tools > Board > Boards Manager**
4. Search for "esp32" and install the ESP32 package by Espressif

### 3. Install Required Libraries

#### TFT_eSPI Library
1. Go to **Sketch > Include Library > Manage Libraries**
2. Search for "TFT_eSPI" and install the library by Bodmer
3. **IMPORTANT**: You must configure the library for the ST7789 display

#### Configure TFT_eSPI for ST7789
1. Navigate to your Arduino libraries folder:
   - **Windows**: `Documents/Arduino/libraries/TFT_eSPI/`
   - **Mac**: `~/Documents/Arduino/libraries/TFT_eSPI/`
   - **Linux**: `~/Arduino/libraries/TFT_eSPI/`

2. Open `User_Setup.h` in a text editor

3. Comment out any existing driver and add these lines:
   ```cpp
   #define ST7789_DRIVER    // Configure for ST7789 display
   
   // Pin definitions (add these to User_Setup.h)
   #define TFT_MISO 19
   #define TFT_MOSI 23
   #define TFT_SCLK 18
   #define TFT_CS   15
   #define TFT_DC    2
   #define TFT_RST   4
   
   #define TFT_WIDTH  240
   #define TFT_HEIGHT 320
   ```

### 4. Choose and Upload Firmware
1. Choose one of the three firmware options:
   - `tonechaser/tonechaser.ino` - Musical demonstration
   - `working/working.ino` - Basic touch wheel test
   - `wheel_visualization/wheel_visualization.ino` - Full interactive demo
2. Open the chosen `.ino` file in Arduino IDE
3. Select your ESP32 board: **Tools > Board > ESP32 Arduino > ESP32 Thing** (or your specific ESP32 variant)
4. Select the correct COM port: **Tools > Port**
5. Click **Upload**

## Firmware Comparison

| Feature | working | tonechaser | wheel_visualization |
|---------|---------|------------|-------------------|
| Touch Detection | ✅ Basic | ✅ Advanced | ✅ Advanced |
| LED Control | ✅ | ✅ | ✅ |
| Audio Feedback | ❌ | ✅ Musical | ✅ Simple |
| Visual Display | ✅ Simple | ✅ Colorful | ✅ Interactive |
| Menu System | ❌ | ❌ | ✅ |
| Multiple Modes | ❌ | ✅ | ✅ |
| Calibration | ✅ | ✅ | ✅ |
| Serial Debug | ✅ | ❌ | ✅ |
| Code Complexity | Simple | Medium | Advanced |

## Usage

### Controls (All Firmware)
- **Touch Wheel**: Touch around the capacitive wheel to see visual feedback and light up LEDs
- **Center Button**: Press the center capacitive button (varies by firmware)
- **Calibration**: All firmware automatically calibrates on startup (don't touch during this phase)

### Specific Features by Firmware

#### `working/` - Basic Test
- Simple touch detection with LED feedback
- Displays raw sensor values and touch angle
- Best for hardware testing and development

#### `tonechaser/` - Musical Explorer
- Multiple musical modes (Free Play, Scale, Chord, Arpeggio)
- Press center button to cycle through modes
- Rich visual effects and musical scales
- Continuous tone generation based on touch position

#### `wheel_visualization/` - Interactive Demo
- Full menu system with navigation
- Touch wheel demo mode
- Placeholder mini-games section
- Credits screen
- Most complete user experience

## Customization

All firmware examples are designed to be easily modified:

- **Pin Configuration**: Modify pin definitions in each sketch
- **Touch Sensitivity**: Adjust touch thresholds
- **Colors and Effects**: Customize visual appearance
- **Audio**: Modify tones and musical scales
- **Menu Items**: Add new screens and functionality

## Troubleshooting

### Display Issues
- **Blank Screen**: Check TFT_eSPI configuration in `User_Setup.h`
- **Wrong Colors**: Verify `ST7789_DRIVER` is defined
- **Upside Down**: Adjust `tft.setRotation(0)` in setup()

### Touch Issues
- **No Touch Response**: Check capacitive touch sensor connections
- **Erratic Behavior**: May need recalibration or threshold adjustment
- **LEDs Not Working**: Verify LED pin connections

### Compilation Errors
- **Library Not Found**: Install TFT_eSPI library via Library Manager
- **Pin Conflicts**: Check pin definitions match your hardware setup
- **Memory Issues**: Try a simpler firmware if running out of memory

## Hardware Notes

This firmware was originally designed for a specific touch wheel badge but can be adapted for custom hardware. The touch wheel uses three capacitive sensors positioned 120° apart to triangulate touch position around a circular perimeter.

The algorithms calculate touch position using trigonometric interpolation between the three sensor readings, providing smooth and accurate position detection around the full 360° wheel.

## Development

Each firmware demonstrates different aspects of the hardware capabilities:

- **`working/`**: Foundation for understanding the basic touch detection algorithms
- **`tonechaser/`**: Advanced audio synthesis and visual effects
- **`wheel_visualization/`**: Complete UI framework and menu system

Use these as starting points for your own projects or as learning examples for capacitive touch wheel implementations.

## Credits

All credit to **Fourfold**. Visit [fourfold.co](https://fourfold.co) for more info.

## License

This is demonstration firmware provided as-is for educational and development purposes. WiFi functionality, splash screens, and proprietary content have been removed for public release.