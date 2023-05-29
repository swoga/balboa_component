#pragma once
#include "esphome/components/select/select.h"
#include "switch.h"

namespace esphome {
namespace balboa {
class BalboaSwitch;

class BalboaLightSelect : public select::Select {
 public:
  void set_light(BalboaSwitch *x) { this->light = x; }

 protected:
  BalboaSwitch *light;
  void control(const std::string &value) override;
};
}  // namespace balboa
}  // namespace esphome