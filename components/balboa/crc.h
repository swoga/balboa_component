#pragma once
#include <stdint.h>

namespace esphome {
namespace balboa {

uint8_t crc8(uint8_t *data, uint8_t length);

}  // namespace balboa
}  // namespace esphome
