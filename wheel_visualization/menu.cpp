#include "menu.h"

MenuSystem::MenuSystem(TFT_eSPI* display) {
  tft = display;
  
  // Initialize menu items internally
  static MenuItem internalMenuItems[] = {
    {"Touch Wheel", WHEEL_SCREEN},
    {"Mini Games", MINI_GAMES_SCREEN},
    {"Credits", CREDITS_SCREEN}
  };
  
  items = internalMenuItems;
  itemCount = sizeof(internalMenuItems) / sizeof(internalMenuItems[0]);
  selectedItem = 0;
  scrollOffset = 0;
  maxVisibleItems = 3;  // Show 3 items at a time
  lastNavTime = 0;
  selectPressed = false;
  
  // Create touch wheel tracker
  wheelTracker = new TouchWheelTracker();
}

void MenuSystem::init() {
  selectedItem = 0;
  scrollOffset = 0;
  selectPressed = false;
  
  // Reset wheel tracker
  if (wheelTracker) {
    wheelTracker->resetRotation();
  }
}

void MenuSystem::draw() {
  // Clear screen
  tft->fillScreen(TFT_BLACK);
  
  // Title - properly centered
  tft->setTextSize(2);
  tft->setTextColor(COLOR_TEXT);
  const char* title = "MENU";
  int titleWidth = strlen(title) * 12; // 12 pixels per character at size 2
  tft->setCursor((240 - titleWidth) / 2, 20);
  tft->println(title);
  
  // Draw menu items with scrolling
  const int itemHeight = 40;
  const int startY = 70;
  
  // Calculate visible range
  int endItem = min(scrollOffset + maxVisibleItems, itemCount);
  
  for (int i = scrollOffset; i < endItem; i++) {
    int displayIndex = i - scrollOffset;
    int y = startY + (displayIndex * itemHeight);
    
    // Draw selection highlight
    if (i == selectedItem) {
      tft->fillRect(20, y - 5, 200, itemHeight - 10, COLOR_SEGMENT_ACTIVE);
      tft->setTextColor(TFT_BLACK);
    } else {
      tft->fillRect(20, y - 5, 200, itemHeight - 10, TFT_BLACK);
      tft->drawRect(20, y - 5, 200, itemHeight - 10, COLOR_WHEEL_OUTLINE);
      tft->setTextColor(COLOR_TEXT);
    }
    
    // Draw menu item text
    tft->setTextSize(2);
    int textWidth = strlen(items[i].name) * 12;
    tft->setCursor((240 - textWidth) / 2, y + 5);
    tft->println(items[i].name);
  }
  
  // Draw scroll indicators
  if (scrollOffset > 0) {
    // Up arrow
    tft->setTextColor(COLOR_SEGMENT_ACTIVE);
    tft->setCursor(110, 50);
    tft->print("^");
  }
  
  if (scrollOffset + maxVisibleItems < itemCount) {
    // Down arrow
    tft->setTextColor(COLOR_SEGMENT_ACTIVE);
    tft->setCursor(110, 195);
    tft->print("v");
  }
  
  // Instructions removed for cleaner look
}

void MenuSystem::drawSelective(int oldSelection, int newSelection) {
  const int itemHeight = 40;
  const int startY = 70;
  
  // Redraw the old selection (unselected style)
  if (oldSelection >= 0 && oldSelection < itemCount) {
    int y = startY + (oldSelection * itemHeight);
    
    // Clear and draw unselected style
    tft->fillRect(20, y - 5, 200, itemHeight - 10, TFT_BLACK);
    tft->drawRect(20, y - 5, 200, itemHeight - 10, COLOR_WHEEL_OUTLINE);
    
    // Draw text
    tft->setTextSize(2);
    tft->setTextColor(COLOR_TEXT);
    int textWidth = strlen(items[oldSelection].name) * 12;
    tft->setCursor((240 - textWidth) / 2, y + 5);
    tft->println(items[oldSelection].name);
  }
  
  // Redraw the new selection (selected style)
  if (newSelection >= 0 && newSelection < itemCount) {
    int y = startY + (newSelection * itemHeight);
    
    // Draw selected style
    tft->fillRect(20, y - 5, 200, itemHeight - 10, COLOR_SEGMENT_ACTIVE);
    
    // Draw text
    tft->setTextSize(2);
    tft->setTextColor(TFT_BLACK);
    int textWidth = strlen(items[newSelection].name) * 12;
    tft->setCursor((240 - textWidth) / 2, y + 5);
    tft->println(items[newSelection].name);
  }
}

void MenuSystem::navigateUp() {
  int oldSelection = selectedItem;
  selectedItem--;
  if (selectedItem < 0) {
    selectedItem = itemCount - 1;  // Wrap to bottom
    scrollOffset = max(0, itemCount - maxVisibleItems);  // Scroll to show bottom
  } else if (selectedItem < scrollOffset) {
    scrollOffset = selectedItem;  // Scroll up to follow selection
  }
  
  // Only redraw if selection actually changed
  if (oldSelection != selectedItem) {
    draw();  // Full redraw for scrolling
    // Feedback tone
    ledcWriteTone(BUZZER_PIN, 440);
    delay(30);
    ledcWriteTone(BUZZER_PIN, 0);
  }
}

void MenuSystem::navigateDown() {
  int oldSelection = selectedItem;
  selectedItem++;
  if (selectedItem >= itemCount) {
    selectedItem = 0;  // Wrap to top
    scrollOffset = 0;  // Scroll to show top
  } else if (selectedItem >= scrollOffset + maxVisibleItems) {
    scrollOffset = selectedItem - maxVisibleItems + 1;  // Scroll down to follow selection
  }
  
  // Only redraw if selection actually changed
  if (oldSelection != selectedItem) {
    draw();  // Full redraw for scrolling
    // Feedback tone
    ledcWriteTone(BUZZER_PIN, 440);
    delay(30);
    ledcWriteTone(BUZZER_PIN, 0);
  }
}

ScreenState MenuSystem::select() {
  // Play selection tone
  ledcWriteTone(BUZZER_PIN, 784);
  delay(100);
  ledcWriteTone(BUZZER_PIN, 0);
  
  return items[selectedItem].targetScreen;
}

// Store baselines
static int menuBaseline1 = 0;
static int menuBaseline2 = 0;
static int menuBaseline3 = 0;

void MenuSystem::setBaselines(int b1, int b2, int b3) {
  menuBaseline1 = b1;
  menuBaseline2 = b2;
  menuBaseline3 = b3;
}

void MenuSystem::handleInput(int touch1, int touch2, int touch3, bool selectButton) {
  // Use the baselines from the main calibration
  if (menuBaseline1 == 0 && menuBaseline2 == 0 && menuBaseline3 == 0) {
    return; // No baselines set yet
  }
  
  // Calculate deviations using the shared baselines
  int dev1 = menuBaseline1 - touch1;
  int dev2 = menuBaseline2 - touch2;
  int dev3 = menuBaseline3 - touch3;
  
  // Apply threshold
  dev1 = (dev1 < 5) ? 0 : dev1;
  dev2 = (dev2 < 5) ? 0 : dev2;
  dev3 = (dev3 < 5) ? 0 : dev3;
  
  bool touched = (dev1 > 0 || dev2 > 0 || dev3 > 0);
  
  // Track angle changes for smooth iPod-like scrolling
  static float lastAngle = -1;
  static bool wasTracking = false;
  static float angleAccumulator = 0;
  
  if (touched) {
    float sum = dev1 + dev2 + dev3;
    if (sum > 10) { // Higher threshold for more stable tracking
      float norm1 = dev1 / sum;
      float norm2 = dev2 / sum;
      float norm3 = dev3 / sum;
      
      float x = norm1 - 0.5f * (norm2 + norm3);
      float y = 0.866f * (norm2 - norm3);
      
      float angle = atan2(y, x) * 180.0f / PI;
      if (angle < 0) angle += 360.0f;
      
      // Track angle changes
      if (wasTracking && lastAngle >= 0) {
        float deltaAngle = angle - lastAngle;
        
        // Handle wrap-around
        if (deltaAngle > 180) deltaAngle -= 360;
        if (deltaAngle < -180) deltaAngle += 360;
        
        // Only accumulate significant changes
        if (abs(deltaAngle) > 3.0 && abs(deltaAngle) < 90.0) { // Ignore huge jumps
          angleAccumulator += deltaAngle;
          
          // Navigate when accumulated enough rotation (30 degrees per item)
          // Reversed: clockwise (positive) = up, counter-clockwise (negative) = down
          if (angleAccumulator >= 30.0) {
            navigateUp();
            angleAccumulator -= 30.0;
          } else if (angleAccumulator <= -30.0) {
            navigateDown();
            angleAccumulator += 30.0;
          }
        }
      }
      
      lastAngle = angle;
      wasTracking = true;
    }
  } else {
    // Reset tracking when not touched
    wasTracking = false;
    angleAccumulator = 0;
  }
  
  // Handle selection
  if (selectButton && !selectPressed) {
    selectPressed = true;
  } else if (!selectButton) {
    selectPressed = false;
  }
}