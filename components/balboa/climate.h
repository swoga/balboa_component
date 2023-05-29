#pragma once
#include "balboa.h"
#include "esphome/components/climate/climate.h"

namespace esphome {
namespace balboa {
class BalboaComponent;

class BalboaClimate : public climate::Climate {
 public:
  void set_parent(BalboaComponent *parent) { this->parent = parent; }

 protected:
  BalboaComponent *parent;
  climate::ClimateTraits traits() override {
    auto traits = climate::ClimateTraits();
    traits.set_supports_current_temperature(true);
    traits.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_HEAT});
    traits.set_visual_temperature_step(0.5);
    traits.set_visual_min_temperature(10.0);
    traits.set_visual_max_temperature(40.0);
    return traits;
  };
  void control(const climate::ClimateCall &call) override;
};
}  // namespace balboa
}  // namespace esphome