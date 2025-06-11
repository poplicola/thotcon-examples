#ifndef TOUCH_WHEEL_TRACKER_H
#define TOUCH_WHEEL_TRACKER_H

#include <Arduino.h>
#include "config.h"

// Touch wheel tracking constants
#define WHEEL_SCAN_RATE_MS 10        // 100Hz scan rate for smooth response
#define TOUCH_THRESHOLD_MIN 10       // Minimum deviation to register touch
#define ANGLE_CHANGE_THRESHOLD 5.0   // Minimum angle change to register movement (degrees)
#define VELOCITY_DECAY_FACTOR 0.92   // Momentum decay per frame
#define VELOCITY_MIN_THRESHOLD 0.5   // Minimum velocity to continue momentum
#define IIR_FILTER_ALPHA 0.3         // Low-pass filter coefficient (0-1, lower = smoother)
#define HYSTERESIS_MARGIN 3          // Degrees of hysteresis to prevent jitter
#define DEBOUNCE_TIME_MS 50          // Debounce time for touch events
#define ACCELERATION_CURVE 1.5       // Non-linear acceleration factor

class TouchWheelTracker {
private:
  // Raw sensor data
  int baseline1, baseline2, baseline3;
  int calibrationSamples;
  bool isCalibrated;
  
  // Touch tracking
  bool wasTouched;
  bool isTouched;
  unsigned long touchStartTime;
  unsigned long lastTouchTime;
  unsigned long lastScanTime;
  
  // Angle tracking with filtering
  float currentAngle;
  float previousAngle;
  float filteredAngle;
  float unwrappedAngle;
  float totalRotation;
  
  // Velocity and momentum
  float angularVelocity;
  float momentumVelocity;
  float velocityHistory[5];
  int velocityIndex;
  
  // Centroid calculation
  float centroidX, centroidY;
  float lastCentroidX, lastCentroidY;
  
  // Dynamic baseline adjustment
  float dynamicBaseline1, dynamicBaseline2, dynamicBaseline3;
  unsigned long lastBaselineUpdate;
  
  // Hysteresis state
  float hysteresisLowThreshold;
  float hysteresisHighThreshold;
  bool inHysteresis;
  
  // Private methods
  void updateCentroid(int dev1, int dev2, int dev3);
  float calculateAngleFromCentroid();
  void applyLowPassFilter(float newAngle);
  void updateVelocity(float deltaAngle, float deltaTime);
  void applyMomentum(float deltaTime);
  void updateDynamicBaseline(int touch1, int touch2, int touch3);
  float applyAccelerationCurve(float velocity);
  bool detectRotation(float deltaAngle);
  
public:
  TouchWheelTracker();
  
  // Calibration
  void startCalibration();
  bool calibrate(int touch1, int touch2, int touch3);
  bool needsCalibration() { return !isCalibrated; }
  
  // Main update cycle
  void update(int touch1, int touch2, int touch3);
  
  // State getters
  bool getTouched() { return isTouched; }
  bool justTouched() { return isTouched && !wasTouched; }
  bool justReleased() { return !isTouched && wasTouched; }
  float getAngle() { return filteredAngle; }
  float getVelocity() { return angularVelocity; }
  float getTotalRotation() { return totalRotation; }
  bool isMoving() { return abs(angularVelocity) > VELOCITY_MIN_THRESHOLD; }
  
  // Menu-specific helpers
  int getRotationSteps(float degreesPerStep = 30.0);
  void resetRotation() { totalRotation = 0; }
  
  // Debug info
  void printDebug();
};

#endif // TOUCH_WHEEL_TRACKER_H