#include "climate.h"

namespace esphome {
namespace balboa {
void BalboaClimate::control(const climate::ClimateCall &call) {
  if (call.get_mode().has_value()) {
    parent->send_toggle_item(ITEM_Restmode);
  }
  if (call.get_target_temperature().has_value()) {
    auto target_temperature = call.get_target_temperature().value();
    // hardcoded to celcius
    uint8_t value = target_temperature * 2;
    uint8_t msg[] = {MSG_SetTargetTemp, value};
    parent->send_buffer(msg, 2);
  }
}

}  // namespace balboa
}  // namespace esphome