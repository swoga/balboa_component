#include "climate.h"

namespace esphome {
namespace balboa {
void BalboaClimate::control(const climate::ClimateCall &call) {
  if (call.get_mode().has_value()) {
    if (this->mode == call.get_mode().value()) {
      ESP_LOGI(TAG, "ignore NOP mode change");
    } else {
      parent->send_toggle_item(ITEM_Restmode);
    }
  }
  if (call.get_target_temperature().has_value()) {
    if (this->target_temperature == call.get_target_temperature().value()) {
      ESP_LOGI(TAG, "ignore NOP target temperature change");
    } else {
      auto target_temperature = call.get_target_temperature().value();
      // hardcoded to celcius
      uint8_t value = target_temperature * 2;
      uint8_t msg[] = {MessageType::SetTargetTemp, value};
      parent->send_buffer(msg, 2);
    }
  }
}

}  // namespace balboa
}  // namespace esphome