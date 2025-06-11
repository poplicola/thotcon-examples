# Touch Wheel Badge - Demo Firmware

This is a cleaned-up demo version of the touch wheel badge firmware, suitable for public release and custom projects.

## Features

- **Touch Wheel Demo**: Interactive capacitive touch wheel with visual feedback
- **LED Indicators**: 6 LEDs that light up based on touch position  
- **Audio Feedback**: Simple tone feedback for interactions
- **Menu System**: Navigate between different modes using the touch wheel
- **Mini Games**: Placeholder for future game implementations
- **Credits**: Information about the original creators

## Hardware Requirements

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

### 4. Upload the Firmware
1. Open `wheel_visualization.ino` in Arduino IDE
2. Select your ESP32 board: **Tools > Board > ESP32 Arduino > ESP32 Thing** (or your specific ESP32 variant)
3. Select the correct COM port: **Tools > Port**
4. Click **Upload**

## Usage

### Controls
- **Touch Wheel**: Touch around the capacitive wheel to see visual feedback and light up LEDs
- **Center Button**: Press the center capacitive button to access the menu system
- **Menu Navigation**: Use the touch wheel to navigate menu items
- **Select**: Press center button to select menu items

### Menu Structure
- **Touch Wheel**: Main touch wheel demonstration mode
- **Mini Games**: Placeholder screen for future game implementations
- **Credits**: Information about the original creators

### Touch Wheel Operation
1. **Calibration**: On first boot, the device calibrates the touch sensors (don't touch during this phase)
2. **Touch Detection**: Touch anywhere around the wheel perimeter to activate LEDs and display feedback
3. **Visual Feedback**: The display shows a colorful wheel with your touch position highlighted
4. **Audio Feedback**: Simple tones provide audio confirmation of interactions

## Customization

This firmware is designed to be easily modified for custom projects:

- **Pin Configuration**: All pins are defined in `config.h` - modify as needed for your hardware
- **Touch Sensitivity**: Adjust `TOUCH_THRESHOLD` in `config.h`
- **Colors**: Modify color definitions in `config.h`
- **Menu Items**: Add new screens by creating classes similar to `CreditsScreen`
- **Touch Algorithms**: Touch processing is in the main `.ino` file for easy modification

## Troubleshooting

### Display Issues
- **Blank Screen**: Check TFT_eSPI configuration in `User_Setup.h`
- **Wrong Colors**: Verify `ST7789_DRIVER` is defined
- **Upside Down**: Adjust `tft.setRotation(0)` in setup()

### Touch Issues
- **No Touch Response**: Check capacitive touch sensor connections
- **Erratic Behavior**: May need recalibration or touch threshold adjustment

### Compilation Errors
- **Library Not Found**: Install TFT_eSPI library via Library Manager
- **Pin Conflicts**: Check pin definitions match your hardware setup

## Hardware Notes

This firmware was originally designed for a specific touch wheel badge but can be adapted for custom hardware. The touch wheel uses three capacitive sensors positioned 120° apart to triangulate touch position around a circular perimeter.

## Credits

All credit to **Fourfold**. Visit [fourfold.co](https://fourfold.co) for more info.

## License

This is demonstration firmware provided as-is for educational and development purposes. WiFi functionality, splash screens, and proprietary content have been removed for public release.