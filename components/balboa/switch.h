#pragma once
#include "balboa.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace balboa {
class BalboaComponent;

class BalboaSwitch : public switch_::Switch {
 public:
  void set_item(uint8_t item) { this->item = item; }
  void set_parent(BalboaComponent *parent);

 protected:
  uint8_t item;
  BalboaComponent *parent;
  void write_state(bool state) override;
};
}  // namespace balboa
}  // namespace esphome
