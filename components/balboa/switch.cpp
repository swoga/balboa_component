#include "switch.h"

namespace esphome {
namespace balboa {

void BalboaSwitch::set_parent(BalboaComponent *parent) { this->parent = parent; }
BalboaComponent *BalboaSwitch::get_parent() { return this->parent; }

void BalboaSwitch::write_state(bool state) {
  if (this->state == state) {
    ESP_LOGI(TAG, "ignore nop state change");
    return;
  }
  send_toggle_item();
}

void BalboaSwitch::send_toggle_item() { parent->send_toggle_item(item); }

}  // namespace balboa
}  // namespace esphome
