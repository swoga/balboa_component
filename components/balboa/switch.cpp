#include "switch.h"

namespace esphome {
namespace balboa {

void BalboaSwitch::set_parent(BalboaComponent *parent) { this->parent = parent; }

void BalboaSwitch::write_state(bool state) { parent->send_toggle_item(item); }

}  // namespace balboa
}  // namespace esphome
