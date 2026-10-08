// CRC-8 used by BMW F-series CAN messages: polynomial 0x1D (SAE J1850), initial value 0xFF,
// no reflection, then XOR with a per-message constant ("final"). The constant for each CAN
// ID was recovered from real captures; see docs/HISTORY.md.
//
// Bitwise implementation on purpose: no 256-byte lookup table, which matters on the Nano's
// 2 KB of RAM. A frame is at most 8 bytes, so the extra cycles don't matter.

#ifndef CRC8_H
#define CRC8_H

#include <Arduino.h>

class CRC8 {
public:
  void begin() {}  // nothing to precompute; kept so existing sketches compile unchanged

  uint8_t get_crc8(const uint8_t *data, int len, uint8_t finalXor) const {
    uint8_t crc = 0xFF;
    for (int i = 0; i < len; i++) {
      crc ^= data[i];
      for (uint8_t bit = 0; bit < 8; bit++)
        crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x1D) : (uint8_t)(crc << 1);
    }
    return crc ^ finalXor;
  }
};

#endif
