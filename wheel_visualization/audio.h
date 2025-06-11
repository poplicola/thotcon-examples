#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include "config.h"

class Audio {
private:
  static bool introPlayed;
  
public:
  static void init();
  // static void playCloseEncounters(); // Removed for public release
  static void playMenuTone();
  static void playSelectTone();
  static void playNavigationTone();
  static void tone(int frequency, int duration);
  static void stopTone();
};

#endif // AUDIO_H