#include "sprites.h"
#include <TFT_eSPI.h>

void drawSprite(TFT_eSPI* tft, const uint8_t* sprite, int x, int y, uint16_t color) {
  for (int py = 0; py < SPRITE_HEIGHT; py++) {
    for (int px = 0; px < SPRITE_WIDTH; px++) {
      // Get byte and bit position
      int byteIndex = (py * SPRITE_WIDTH + px) / 8;
      int bitIndex = 7 - ((py * SPRITE_WIDTH + px) % 8);
      
      // Read pixel from PROGMEM
      uint8_t pixelByte = pgm_read_byte(&sprite[byteIndex]);
      bool pixelOn = (pixelByte >> bitIndex) & 0x01;
      
      // Draw pixel if on
      if (pixelOn) {
        tft->drawPixel(x + px, y + py, color);
      }
    }
  }
}