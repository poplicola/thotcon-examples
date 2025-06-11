#include "audio.h"

bool Audio::introPlayed = false;

void Audio::init() {
  ledcAttach(BUZZER_PIN, 1000, 8);
}

// Close Encounters music removed for public release
/*
void Audio::playCloseEncounters() {
  // Function removed
}
*/

void Audio::playMenuTone() {
  tone(523, 50);  // C5
  delay(50);
  tone(659, 50);  // E5
  delay(50);
  stopTone();
}

void Audio::playSelectTone() {
  tone(784, 100);  // G5
  delay(100);
  stopTone();
}

void Audio::playNavigationTone() {
  tone(440, 30);
  delay(30);
  stopTone();
}

void Audio::tone(int frequency, int duration) {
  ledcWriteTone(BUZZER_PIN, frequency);
  if (duration > 0) {
    delay(duration);
  }
}

void Audio::stopTone() {
  ledcWriteTone(BUZZER_PIN, 0);
}