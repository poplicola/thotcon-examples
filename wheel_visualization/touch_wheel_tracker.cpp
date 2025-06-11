#include "touch_wheel_tracker.h"

TouchWheelTracker::TouchWheelTracker() {
  // Initialize all variables
  baseline1 = baseline2 = baseline3 = 0;
  calibrationSamples = 0;
  isCalibrated = false;
  
  wasTouched = false;
  isTouched = false;
  touchStartTime = 0;
  lastTouchTime = 0;
  lastScanTime = 0;
  
  currentAngle = 0;
  previousAngle = 0;
  filteredAngle = 0;
  unwrappedAngle = 0;
  totalRotation = 0;
  
  angularVelocity = 0;
  momentumVelocity = 0;
  velocityIndex = 0;
  for (int i = 0; i < 5; i++) {
    velocityHistory[i] = 0;
  }
  
  centroidX = centroidY = 0;
  lastCentroidX = lastCentroidY = 0;
  
  dynamicBaseline1 = dynamicBaseline2 = dynamicBaseline3 = 0;
  lastBaselineUpdate = 0;
  
  hysteresisLowThreshold = -HYSTERESIS_MARGIN;
  hysteresisHighThreshold = HYSTERESIS_MARGIN;
  inHysteresis = false;
}

void TouchWheelTracker::startCalibration() {
  calibrationSamples = 0;
  isCalibrated = false;
  baseline1 = baseline2 = baseline3 = 0;
}

bool TouchWheelTracker::calibrate(int touch1, int touch2, int touch3) {
  if (calibrationSamples < CALIBRATION_SAMPLES) {
    if (calibrationSamples == 0) {
      baseline1 = touch1;
      baseline2 = touch2;
      baseline3 = touch3;
      dynamicBaseline1 = touch1;
      dynamicBaseline2 = touch2;
      dynamicBaseline3 = touch3;
    } else {
      // Running average for calibration
      baseline1 = (baseline1 * calibrationSamples + touch1) / (calibrationSamples + 1);
      baseline2 = (baseline2 * calibrationSamples + touch2) / (calibrationSamples + 1);
      baseline3 = (baseline3 * calibrationSamples + touch3) / (calibrationSamples + 1);
    }
    calibrationSamples++;
    
    if (calibrationSamples >= CALIBRATION_SAMPLES) {
      isCalibrated = true;
      return true;
    }
  }
  return false;
}

void TouchWheelTracker::update(int touch1, int touch2, int touch3) {
  if (!isCalibrated) return;
  
  unsigned long now = millis();
  float deltaTime = (now - lastScanTime) / 1000.0f;
  if (deltaTime < WHEEL_SCAN_RATE_MS / 1000.0f) return; // Rate limiting
  
  lastScanTime = now;
  
  // Calculate deviations from baseline
  int dev1 = baseline1 - touch1;
  int dev2 = baseline2 - touch2;
  int dev3 = baseline3 - touch3;
  
  // Apply minimum threshold
  dev1 = (abs(dev1) < TOUCH_THRESHOLD_MIN) ? 0 : dev1;
  dev2 = (abs(dev2) < TOUCH_THRESHOLD_MIN) ? 0 : dev2;
  dev3 = (abs(dev3) < TOUCH_THRESHOLD_MIN) ? 0 : dev3;
  
  // Update touch state
  wasTouched = isTouched;
  isTouched = (dev1 > 0 || dev2 > 0 || dev3 > 0);
  
  // Handle touch events
  if (justTouched()) {
    touchStartTime = now;
    lastTouchTime = now;
    // Reset momentum when starting new touch
    momentumVelocity = 0;
    angularVelocity = 0;
    totalRotation = 0;
  }
  
  if (isTouched) {
    lastTouchTime = now;
    
    // Update centroid
    updateCentroid(dev1, dev2, dev3);
    
    // Calculate angle from centroid
    float newAngle = calculateAngleFromCentroid();
    
    // Apply low-pass filter
    applyLowPassFilter(newAngle);
    
    // Calculate angular change
    float deltaAngle = filteredAngle - previousAngle;
    
    // Handle angle wrapping (crossing 0/360 boundary)
    if (deltaAngle > 180) deltaAngle -= 360;
    if (deltaAngle < -180) deltaAngle += 360;
    
    // Detect if this is actual rotation or just noise
    if (detectRotation(deltaAngle)) {
      // Update velocity
      updateVelocity(deltaAngle, deltaTime);
      
      // Accumulate total rotation
      totalRotation += deltaAngle;
      
      previousAngle = filteredAngle;
    }
    
    // Update dynamic baseline slowly when touched
    updateDynamicBaseline(touch1, touch2, touch3);
    
  } else {
    // Not touched - apply momentum scrolling
    if (abs(momentumVelocity) > VELOCITY_MIN_THRESHOLD) {
      applyMomentum(deltaTime);
      totalRotation += momentumVelocity * deltaTime;
    } else {
      angularVelocity = 0;
      momentumVelocity = 0;
    }
  }
}

void TouchWheelTracker::updateCentroid(int dev1, int dev2, int dev3) {
  float sum = dev1 + dev2 + dev3;
  if (sum < TOUCH_THRESHOLD_MIN) {
    centroidX = lastCentroidX;
    centroidY = lastCentroidY;
    return;
  }
  
  // Weighted average for centroid position
  float norm1 = dev1 / sum;
  float norm2 = dev2 / sum;
  float norm3 = dev3 / sum;
  
  // Convert to X,Y coordinates (sensors at 0°, 120°, 240°)
  centroidX = norm1 - 0.5f * (norm2 + norm3);
  centroidY = 0.866f * (norm2 - norm3);
  
  // Smooth centroid movement
  if (lastCentroidX != 0 || lastCentroidY != 0) {
    centroidX = lastCentroidX + (centroidX - lastCentroidX) * 0.5f;
    centroidY = lastCentroidY + (centroidY - lastCentroidY) * 0.5f;
  }
  
  lastCentroidX = centroidX;
  lastCentroidY = centroidY;
}

float TouchWheelTracker::calculateAngleFromCentroid() {
  float angle = atan2(centroidY, centroidX) * 180.0f / PI;
  if (angle < 0) angle += 360.0f;
  return angle;
}

void TouchWheelTracker::applyLowPassFilter(float newAngle) {
  // Handle angle wrapping for filter
  float delta = newAngle - filteredAngle;
  if (delta > 180) delta -= 360;
  if (delta < -180) delta += 360;
  
  // IIR low-pass filter
  filteredAngle += delta * IIR_FILTER_ALPHA;
  
  // Normalize angle
  if (filteredAngle < 0) filteredAngle += 360;
  if (filteredAngle >= 360) filteredAngle -= 360;
}

void TouchWheelTracker::updateVelocity(float deltaAngle, float deltaTime) {
  if (deltaTime <= 0) return;
  
  // Calculate instantaneous velocity
  float instantVelocity = deltaAngle / deltaTime;
  
  // Store in circular buffer for averaging
  velocityHistory[velocityIndex] = instantVelocity;
  velocityIndex = (velocityIndex + 1) % 5;
  
  // Calculate average velocity
  float avgVelocity = 0;
  for (int i = 0; i < 5; i++) {
    avgVelocity += velocityHistory[i];
  }
  avgVelocity /= 5.0f;
  
  // Apply acceleration curve
  angularVelocity = applyAccelerationCurve(avgVelocity);
  
  // Set momentum for when touch is released
  momentumVelocity = angularVelocity;
}

void TouchWheelTracker::applyMomentum(float deltaTime) {
  // Exponential decay
  momentumVelocity *= VELOCITY_DECAY_FACTOR;
  
  // Stop if below threshold
  if (abs(momentumVelocity) < VELOCITY_MIN_THRESHOLD) {
    momentumVelocity = 0;
  }
  
  angularVelocity = momentumVelocity;
}

void TouchWheelTracker::updateDynamicBaseline(int touch1, int touch2, int touch3) {
  unsigned long now = millis();
  
  // Update baseline slowly to compensate for drift
  if (now - lastBaselineUpdate > 1000) { // Every second
    float alpha = 0.01f; // Very slow adaptation
    dynamicBaseline1 = dynamicBaseline1 * (1 - alpha) + touch1 * alpha;
    dynamicBaseline2 = dynamicBaseline2 * (1 - alpha) + touch2 * alpha;
    dynamicBaseline3 = dynamicBaseline3 * (1 - alpha) + touch3 * alpha;
    lastBaselineUpdate = now;
  }
}

float TouchWheelTracker::applyAccelerationCurve(float velocity) {
  // Non-linear acceleration curve for natural feel
  float sign = (velocity < 0) ? -1.0f : 1.0f;
  float magnitude = abs(velocity);
  
  // Apply power curve
  magnitude = pow(magnitude / 100.0f, ACCELERATION_CURVE) * 100.0f;
  
  return sign * magnitude;
}

bool TouchWheelTracker::detectRotation(float deltaAngle) {
  float absDelta = abs(deltaAngle);
  
  // Hysteresis to prevent jitter
  if (!inHysteresis) {
    if (absDelta > ANGLE_CHANGE_THRESHOLD) {
      inHysteresis = true;
      return true;
    }
  } else {
    if (absDelta < ANGLE_CHANGE_THRESHOLD - HYSTERESIS_MARGIN) {
      inHysteresis = false;
      return false;
    }
    return true;
  }
  
  return false;
}

int TouchWheelTracker::getRotationSteps(float degreesPerStep) {
  int steps = (int)(totalRotation / degreesPerStep);
  if (steps != 0) {
    totalRotation -= steps * degreesPerStep;
  }
  return steps;
}

void TouchWheelTracker::printDebug() {
  Serial.print("Touch: ");
  Serial.print(isTouched);
  Serial.print(" Angle: ");
  Serial.print(filteredAngle);
  Serial.print(" Velocity: ");
  Serial.print(angularVelocity);
  Serial.print(" Total: ");
  Serial.println(totalRotation);
}