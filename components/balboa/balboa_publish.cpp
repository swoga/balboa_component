#include "balboa.h"

namespace esphome {
namespace balboa {

void BalboaComponent::publish(binary_sensor::BinarySensor *sensor, bool value, bool change_only) {
  if (!sensor) {
    return;
  }
  if (change_only && sensor->has_state() && sensor->state == value) {
    return;
  }
  sensor->publish_state(value);
}

void BalboaComponent::publish(BalboaSwitch *sensor, bool value, bool change_only) {
  if (!sensor) {
    return;
  }
  if (change_only && sensor->state == value) {
    return;
  }
  sensor->publish_state(value);
}

void BalboaComponent::publish(BalboaClimate *sensor, climate::ClimateMode mode, float current_temperature,
                              float target_temperature, bool change_only) {
  if (!sensor) {
    return;
  }

  bool any_change = !change_only;

  any_change |= sensor->current_temperature != current_temperature;
  sensor->current_temperature = current_temperature;

  any_change |= sensor->mode != mode;
  sensor->mode = mode;

  any_change |= sensor->target_temperature != target_temperature;
  sensor->target_temperature = target_temperature;

  if (!any_change) {
    return;
  }

  sensor->publish_state();
};

}  // namespace balboa
}  // namespace esphome