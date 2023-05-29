#include "crc.h"

namespace esphome {
namespace balboa {

uint8_t crc8(uint8_t *data, uint8_t length) {
  unsigned long crc;
  int bit;

  crc = 0x02;
  for (int i = 0; i < length; i++) {
    crc ^= data[i];
    for (bit = 0; bit < 8; bit++) {
      if ((crc & 0x80) != 0) {
        crc <<= 1;
        crc ^= 0x7;
      } else {
        crc <<= 1;
      }
    }
  }

  return crc ^ 0x02;
}

}  // namespace balboa
}  // namespace esphome