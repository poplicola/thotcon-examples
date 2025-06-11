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

// Game mode constants
#define MODE_FREE_PLAY 0
#define MODE_SCALE 1
#define MODE_CHORD 2
#define MODE_ARPEGGIO 3
#define NUM_MODES 4

// Base frequencies for musical notes (A4 = 440Hz standard tuning)
const float NOTE_C4 = 261.63;
const float NOTE_CS4 = 277.18;
const float NOTE_D4 = 293.66;
const float NOTE_DS4 = 311.13;
const float NOTE_E4 = 329.63;
const float NOTE_F4 = 349.23;
const float NOTE_FS4 = 369.99;
const float NOTE_G4 = 392.00;
const float NOTE_GS4 = 415.30;
const float NOTE_A4 = 440.00;
const float NOTE_AS4 = 466.16;
const float NOTE_B4 = 493.88;
const float NOTE_C5 = 523.25;

// Different musical scales
const float SCALE_MAJOR[] = {1.0, 1.122, 1.26, 1.334, 1.498, 1.682, 1.887, 2.0}; // Major scale intervals
const float SCALE_MINOR[] = {1.0, 1.122, 1.189, 1.334, 1.498, 1.587, 1.782, 2.0}; // Natural minor scale intervals
const float SCALE_PENTA[] = {1.0, 1.122, 1.26, 1.498, 1.682, 2.0}; // Pentatonic scale intervals
const float SCALE_BLUES[] = {1.0, 1.122, 1.189, 1.26, 1.334, 1.498, 1.682, 2.0}; // Blues scale intervals

// Game state variables
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite wheelSprite = TFT_eSprite(&tft); // Sprite for double buffering

// Global variables for wheel visualization
const int wheelCenterX = 120; // Centered on screen
const int wheelCenterY = 150;
const int wheelRadius = 80;   // Larger wheel
const int innerWheelRadius = 55;

// Define colors for the wheel
const uint16_t SECTOR_COLORS[6] = {
  TFT_RED,       // Sector 1 (C)
  TFT_ORANGE,    // Sector 2 (D)
  TFT_YELLOW,    // Sector 3 (E)
  TFT_GREEN,     // Sector 4 (F)
  TFT_BLUE,      // Sector 5 (G)
  TFT_PURPLE     // Sector 6 (A)
};

// HSV to RGB conversion for smooth color transitions
uint16_t getColorFromHSV(float h, float s, float v) {
  // h: 0-360, s: 0-1, v: 0-1
  float c = v * s;
  float x = c * (1 - abs(fmod(h / 60.0, 2) - 1));
  float m = v - c;
  float r, g, b;
  
  if(h >= 0 && h < 60) { r = c; g = x; b = 0; }
  else if(h >= 60 && h < 120) { r = x; g = c; b = 0; }
  else if(h >= 120 && h < 180) { r = 0; g = c; b = x; }
  else if(h >= 180 && h < 240) { r = 0; g = x; b = c; }
  else if(h >= 240 && h < 300) { r = x; g = 0; b = c; }
  else { r = c; g = 0; b = x; }
  
  uint8_t red = (r + m) * 255;
  uint8_t green = (g + m) * 255;
  uint8_t blue = (b + m) * 255;
  
  // Convert RGB to 565 color format
  return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
}

// Variables for touch processing
int touch1, touch2, touch3;
int baseline1, baseline2, baseline3;
bool calibrated = false;
int calibrationCount = 0;
const int CALIBRATION_SAMPLES = 100;

// Variables for continuous tone generation
float currentFrequency = 0;
bool isToneActive = false;
unsigned long lastTouchTime = 0;
const unsigned long TOUCH_TIMEOUT = 2000; // LED timeout in ms
float touchIntensity = 0; // For touch pressure visualization

// For polar angle calculation
float lastAngle = 0;
float currentAngle = 0;
float exactLedPosition = 0; // 0.0 to 5.999...

// Game mode variables
int currentMode = MODE_FREE_PLAY;
int currentScale = 0; // 0=Major, 1=Minor, 2=Pentatonic, 3=Blues
float baseFrequency = NOTE_C4; // Default starting note

// For visual effects
float rippleRadius = 0;
bool isRippling = false;
unsigned long rippleStartTime = 0;

// Variables for button handling
bool buttonPressed = false;
bool lastButtonPressed = false;
unsigned long lastModeChangeTime = 0;
const unsigned long MODE_CHANGE_DEBOUNCE = 500; // Debounce time for mode changes

// For chord mode
bool chordActive = false;
int chordRoot = 0;
const int CHORD_TIMEOUT = 2000;
unsigned long chordStartTime = 0;

// Function declarations
void turnOffAllLEDs();
void updateLEDsSmooth(float position);
void playToneSmooth(float position);
void stopTone();
float calculateAngle(int dev1, int dev2, int dev3);
float angleToLedPosition(float angle);
void drawWheel(float highlightPosition, float intensity);
void showModeName(int mode);
void performCalibration();
void processTouchInput();
void handleModeChange();
void drawRippleEffect();
void playChord(int root);
void playArpeggio(int root);
float getScaledFrequency(float position);

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Tone Explorer");
  
  // Set up LED pins
  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);
  pinMode(LED_3_PIN, OUTPUT);
  pinMode(LED_4_PIN, OUTPUT);
  pinMode(LED_5_PIN, OUTPUT);
  pinMode(LED_6_PIN, OUTPUT);
  
  // Turn off all LEDs initially
  turnOffAllLEDs();
  
  // Set up buzzer
  ledcAttach(BUZZER_PIN, 1000, 8);
  
  // Initialize display
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  
  // Initialize the sprite for double buffering
  // Create a sprite that's large enough for the wheel
  wheelSprite.createSprite(wheelRadius * 2 + 20, wheelRadius * 2 + 20);
  wheelSprite.fillSprite(TFT_BLACK);
  
  // Display minimal initial message
  tft.setCursor(10, 10);
  tft.println("Tone Explorer");
  tft.setCursor(10, 30);
  tft.println("Calibrating...");

  // Initialize random seed
  randomSeed(analogRead(A0));
}

void loop() {
  // Read touch values (less frequently to reduce flicker)
  static unsigned long lastTouchReadTime = 0;
  if (millis() - lastTouchReadTime > 15) { // Read touch every 15ms
    touch1 = touchRead(Q1_PIN);
    touch2 = touchRead(Q2_PIN);
    touch3 = touchRead(Q3_PIN);
    int selectTouch = touchRead(SELECT_PIN);
    
    // Check if center button is pressed
    buttonPressed = (selectTouch < TOUCH_THRESHOLD);
    lastTouchReadTime = millis();
  }
  
  // Perform initial calibration
  if (!calibrated) {
    performCalibration();
    return;
  }

  // Handle mode change button
  if (buttonPressed && !lastButtonPressed) {
    if (millis() - lastModeChangeTime > MODE_CHANGE_DEBOUNCE) {
      handleModeChange();
      lastModeChangeTime = millis();
    }
  }

  // Process touch input
  processTouchInput();
  
  // Draw ripple effect if active
  if (isRippling) {
    drawRippleEffect();
  }
  
  // Handle chord timeouts in chord mode
  if (currentMode == MODE_CHORD && chordActive) {
    if (millis() - chordStartTime > CHORD_TIMEOUT) {
      chordActive = false;
      stopTone();
    }
  }
  
  // Store button state for edge detection
  lastButtonPressed = buttonPressed;
  
  // Small delay to prevent CPU hogging and reduce screen refresh rate
  delay(30); // Increased from 10ms to 30ms to reduce flickering
}

// Perform initial touch wheel calibration
void performCalibration() {
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
  } else {
    calibrated = true;
    
    // Clear screen and show minimal UI
    tft.fillScreen(TFT_BLACK);
    
    // Draw only essential text
    tft.setCursor(10, 10);
    tft.println("Tone Explorer");
    
    tft.setCursor(10, 30);
    tft.println("Press button to change mode");
    
    // Show current mode
    showModeName(currentMode);
    
    // Draw the wheel visualization (centered and larger)
    drawWheel(-1, 0);
  }
}

// Process touch input and calculate wheel position
void processTouchInput() {
  static float lastProcessedPosition = -100; // Initial value to force first update
  
  // Calculate deviations from baseline
  int deviation1 = baseline1 - touch1;
  int deviation2 = baseline2 - touch2;
  int deviation3 = baseline3 - touch3;
  
  // Calculate touch intensity (for visual effects)
  touchIntensity = (deviation1 + deviation2 + deviation3) / 30.0;
  touchIntensity = constrain(touchIntensity, 0, 1);
  
  // Ignore small deviations
  deviation1 = (deviation1 < 5) ? 0 : deviation1;
  deviation2 = (deviation2 < 5) ? 0 : deviation2;
  deviation3 = (deviation3 < 5) ? 0 : deviation3;
  
  // Detect if wheel is being touched
  bool touched = (deviation1 > 5 || deviation2 > 5 || deviation3 > 5);
  
  if (touched) {
    lastTouchTime = millis();
    
    // Convert the three sensor values to a polar angle
    float angle = calculateAngle(deviation1, deviation2, deviation3);
    currentAngle = angle;
    
    // Get exact LED position (0.0 to 5.999...)
    exactLedPosition = angleToLedPosition(angle);
    
    // Only process significant position changes to reduce screen updates
    if (abs(exactLedPosition - lastProcessedPosition) > 0.05) {
      lastProcessedPosition = exactLedPosition;
      
      // Handle modes
      switch (currentMode) {
        case MODE_FREE_PLAY:
          // Free play mode - continuous tone based on exact position
          playToneSmooth(exactLedPosition);
          break;
          
        case MODE_SCALE:
          // Scale mode - quantize to scale notes
          playToneSmooth(floor(exactLedPosition));
          break;
          
        case MODE_CHORD:
          // Chord mode - play chord when a new position is touched
          if ((int)exactLedPosition != chordRoot || !chordActive) {
            chordRoot = (int)exactLedPosition;
            playChord(chordRoot);
            chordActive = true;
            chordStartTime = millis();
            
            // Trigger ripple effect
            isRippling = true;
            rippleRadius = 0;
            rippleStartTime = millis();
          }
          break;
          
        case MODE_ARPEGGIO:
          // Arpeggio mode - play arpeggios based on position
          playArpeggio((int)exactLedPosition);
          break;
      }
      
      // Update LEDs based on continuous position
      updateLEDsSmooth(exactLedPosition);
      
      // Update visualization on the display
      drawWheel(exactLedPosition, touchIntensity);
    }
  } else {
    // If no touch for TOUCH_TIMEOUT ms, turn off tone and LEDs
    if (millis() - lastTouchTime > TOUCH_TIMEOUT && isToneActive) {
      stopTone();
      turnOffAllLEDs();
      isToneActive = false;
      
      // Redraw wheel with no highlight
      if (lastProcessedPosition >= 0) {
        lastProcessedPosition = -1;
        drawWheel(-1, 0);
      }
    }
  }
}

// Handle mode change
void handleModeChange() {
  // Change to next mode
  currentMode = (currentMode + 1) % NUM_MODES;
  
  // Stop any active tones
  stopTone();
  
  // Show the new mode name
  showModeName(currentMode);
  
  // Special case for chord mode
  if (currentMode == MODE_CHORD) {
    chordActive = false;
  }
}

// Display current mode name
void showModeName(int mode) {
  // Clear the mode area
  tft.fillRect(10, 50, tft.width() - 20, 30, TFT_BLACK);
  
  tft.setCursor(10, 50);
  tft.setTextSize(2);
  
  switch (mode) {
    case MODE_FREE_PLAY:
      tft.print("Free Play");
      break;
    case MODE_SCALE:
      tft.print("Scale Mode");
      break;
    case MODE_CHORD:
      tft.print("Chord Mode");
      break;
    case MODE_ARPEGGIO:
      tft.print("Arpeggio");
      break;
  }
  
  tft.setTextSize(1);
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

// Convert angle to LED position (0.0-5.999...)
float angleToLedPosition(float angle) {
  // Map 360 degrees to 6 positions (each 60 degrees)
  // Add 30 for proper sector alignment, then divide by 60 to get position
  float position = (angle + 30) / 60;
  while (position >= 6) position -= 6; // Handle wrap-around
  
  // Flip the LED position (0→5, 1→4, 2→3, 3→2, 4→1, 5→0)
  return 5 - position;
}

// Get a scaled frequency based on position and current scale
float getScaledFrequency(float position) {
  // Determine base frequency and scale based on mode
  float freq;
  int pos = (int)floor(position) % 6; // Get the base position (0-5)
  float fraction = position - floor(position); // Get fractional part
  
  // Start with C major scale base notes
  float baseNotes[6] = {
    NOTE_C4,  // Position 0
    NOTE_D4,  // Position 1
    NOTE_E4,  // Position 2
    NOTE_F4,  // Position 3
    NOTE_G4,  // Position 4
    NOTE_A4   // Position 5
  };
  
  // Base frequency from position
  baseFrequency = baseNotes[pos];
  
  // In free play mode, allow smooth transitions
  if (currentMode == MODE_FREE_PLAY) {
    // If we're between positions, interpolate between the two notes
    if (fraction > 0) {
      int nextPos = (pos + 1) % 6;
      freq = baseNotes[pos] * (1 - fraction) + baseNotes[nextPos] * fraction;
    } else {
      freq = baseNotes[pos];
    }
  } else {
    // In other modes, use the scaled frequency
    freq = baseFrequency;
  }
  
  return freq;
}

// Play a smooth continuous tone based on position
void playToneSmooth(float position) {
  float frequency = getScaledFrequency(position);
  
  // Set the tone
  ledcWriteTone(BUZZER_PIN, frequency);
  currentFrequency = frequency;
  isToneActive = true;
}

// Play a chord based on root position
void playChord(int root) {
  // Stop any previous tone
  ledcWriteTone(BUZZER_PIN, 0);
  delay(10);
  
  // Get base frequency for the root
  float rootFreq = getScaledFrequency(root);
  
  // Play chord tones in sequence
  ledcWriteTone(BUZZER_PIN, rootFreq);
  delay(50);
  ledcWriteTone(BUZZER_PIN, rootFreq * 1.26); // Major third
  delay(50);
  ledcWriteTone(BUZZER_PIN, rootFreq * 1.5);  // Perfect fifth
  delay(50);
  
  // Play all together (simulated chord)
  for(int i = 0; i < 5; i++) {
    ledcWriteTone(BUZZER_PIN, rootFreq);
    delay(10);
    ledcWriteTone(BUZZER_PIN, rootFreq * 1.26);
    delay(10);
    ledcWriteTone(BUZZER_PIN, rootFreq * 1.5);
    delay(10);
  }
  
  // Continue with root note
  ledcWriteTone(BUZZER_PIN, rootFreq);
}

// Play arpeggio based on root position
void playArpeggio(int root) {
  static unsigned long lastArpTime = 0;
  static int arpStep = 0;
  static int lastRoot = -1;
  
  // If root changed, reset arpeggio
  if (root != lastRoot) {
    arpStep = 0;
    lastRoot = root;
  }
  
  // Get base frequency for the root
  float rootFreq = getScaledFrequency(root);
  
  // Define arpeggio pattern intervals
  float arpIntervals[4] = {1.0, 1.26, 1.5, 2.0}; // Root, third, fifth, octave
  
  // Play arpeggio tones in sequence, one every 150ms
  if (millis() - lastArpTime > 150) {
    ledcWriteTone(BUZZER_PIN, rootFreq * arpIntervals[arpStep]);
    arpStep = (arpStep + 1) % 4;
    lastArpTime = millis();
    
    // Trigger a small ripple effect
    isRippling = true;
    rippleRadius = 0;
    rippleStartTime = millis();
  }
}

// Stop the current tone
void stopTone() {
  ledcWriteTone(BUZZER_PIN, 0);
  isToneActive = false;
}

// Update LEDs based on continuous position
void updateLEDsSmooth(float position) {
  // Always start with all LEDs off
  turnOffAllLEDs();
  
  // Get the integer position and fractional part
  int pos = (int)floor(position) % 6;
  float fraction = position - floor(position);
  
  // For regular modes, light up the LED at the integer position
  switch (pos) {
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

// Draw the wheel visualization with continuous highlight using double buffering
void drawWheel(float highlightPosition, float intensity) {
  static float lastHighlightPosition = -2; // Initialize to a value different from default -1
  static float lastIntensity = -1;
  
  // Only redraw if something changed
  if (abs(highlightPosition - lastHighlightPosition) < 0.1 && abs(intensity - lastIntensity) < 0.1) {
    return; // Skip redrawing if position and intensity haven't changed enough
  }
  
  // Store current values for next comparison
  lastHighlightPosition = highlightPosition;
  lastIntensity = intensity;
  
  // Clear the sprite
  wheelSprite.fillSprite(TFT_BLACK);
  
  // Calculate sprite center coordinates
  int spriteWheelCenterX = wheelSprite.width() / 2;
  int spriteWheelCenterY = wheelSprite.height() / 2;
  
  // Draw each sector with its color
  for (int sector = 0; sector < 6; sector++) {
    // Convert LED position to wheel sector (they're flipped)
    int wheelSector = 5 - sector;
    
    // Calculate start and end angles for this sector
    float startAngle = wheelSector * 60.0f;
    float endAngle = startAngle + 60.0f;
    
    // Determine the color
    uint16_t color;
    
    // Check if this sector should be highlighted
    bool isHighlighted = false;
    if (highlightPosition >= 0) {
      int highlightSector = (int)highlightPosition;
      if (sector == highlightSector) {
        isHighlighted = true;
      }
    }
    
    // For highlighted sector, use a more vibrant color based on touch intensity
    if (isHighlighted) {
      color = getColorFromHSV(360.0 * sector / 6.0, 1.0, 0.5 + 0.5 * intensity);
    } else {
      color = SECTOR_COLORS[sector];
    }
    
    // Adjust coordinates for sprite (center the wheel in the sprite)
    int spriteWheelCenterX = wheelSprite.width() / 2;
    int spriteWheelCenterY = wheelSprite.height() / 2;
    
    // Draw the sector
    for (float angle = startAngle; angle <= endAngle; angle += 0.5) {
      float rad = angle * PI / 180.0;
      // Flip x-coordinate to match the working example's orientation
      int x = spriteWheelCenterX - cos(rad) * wheelRadius;
      int y = spriteWheelCenterY + sin(rad) * wheelRadius;
      int innerX = spriteWheelCenterX - cos(rad) * innerWheelRadius;
      int innerY = spriteWheelCenterY + sin(rad) * innerWheelRadius;
      
      // Draw a line from inner to outer radius
      wheelSprite.drawLine(innerX, innerY, x, y, color);
    }
    
    // Add sector number label
    float midAngle = (startAngle + endAngle) / 2;
    float midRad = midAngle * PI / 180.0;
    // Flip x-coordinate to match the working example's orientation
    int labelX = spriteWheelCenterX - cos(midRad) * (wheelRadius * 0.7);
    int labelY = spriteWheelCenterY + sin(midRad) * (wheelRadius * 0.7);
    
    // Draw a small circle with the sector number
    wheelSprite.fillCircle(labelX, labelY, 8, TFT_BLACK);
    wheelSprite.drawCircle(labelX, labelY, 8, TFT_WHITE);
    
    // Write the original sector number + 1 (for human-readable labeling from 1-6)
    wheelSprite.setCursor(labelX - 3, labelY - 3);
    wheelSprite.setTextColor(TFT_WHITE);
    wheelSprite.print(sector + 1);
  }
  
  // Draw wheel outline
  wheelSprite.drawCircle(spriteWheelCenterX, spriteWheelCenterY, wheelRadius, TFT_WHITE);
  wheelSprite.drawCircle(spriteWheelCenterX, spriteWheelCenterY, innerWheelRadius, TFT_WHITE);
  
  // Draw touch indicator at the current angle if wheel is being touched
  if (highlightPosition >= 0) {
    // Convert the LED position back to an angle for visualization
    // LED positions are 0-5, so multiply by 60 degrees and subtract 30 for alignment
    float touchAngle = (5 - highlightPosition) * 60.0 - 30.0;
    if (touchAngle < 0) touchAngle += 360.0;
    
    float rad = touchAngle * PI / 180.0;
    // Flip x-coordinate to match the working example's orientation
    int touchX = spriteWheelCenterX - cos(rad) * wheelRadius;
    int touchY = spriteWheelCenterY + sin(rad) * wheelRadius;
    
    // Draw touch indicator
    wheelSprite.fillCircle(touchX, touchY, 5 + (intensity * 5), TFT_WHITE);
  }
  
  // Push the sprite to the display at the wheel position
  wheelSprite.pushSprite(wheelCenterX - spriteWheelCenterX, wheelCenterY - spriteWheelCenterY);
}

// Draw ripple effect
void drawRippleEffect() {
  // Calculate ripple properties
  unsigned long rippleAge = millis() - rippleStartTime;
  float maxRadius = wheelRadius * 1.5;
  
  // Ripple grows and fades
  rippleRadius = (rippleAge / 300.0) * maxRadius;
  int rippleOpacity = 255 - (rippleAge / 2);
  
  // End ripple animation after it grows beyond max radius or becomes transparent
  if (rippleRadius >= maxRadius || rippleOpacity <= 0) {
    isRippling = false;
    return;
  }
  
  // Clear sprite and redraw the wheel with current highlight
  drawWheel(exactLedPosition, touchIntensity);
  
  // Adjust coordinates for sprite (center the wheel in the sprite)
  int spriteWheelCenterX = wheelSprite.width() / 2;
  int spriteWheelCenterY = wheelSprite.height() / 2;
  
  // Draw the ripple as a circle
  uint16_t rippleColor = ((rippleOpacity & 0xF8) << 8) | ((rippleOpacity & 0xFC) << 3) | (rippleOpacity >> 3);
  wheelSprite.drawCircle(spriteWheelCenterX, spriteWheelCenterY, rippleRadius, rippleColor);
  
  // Draw a second ripple slightly offset for thickness
  wheelSprite.drawCircle(spriteWheelCenterX, spriteWheelCenterY, rippleRadius - 1, rippleColor);
  
  // Push the sprite to the display
  wheelSprite.pushSprite(wheelCenterX - spriteWheelCenterX, wheelCenterY - spriteWheelCenterY);
}

// Helper function to map float value from one range to another
float map(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
  return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}