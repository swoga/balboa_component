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

  bool changed_current_temperature = sensor->current_temperature != current_temperature;
  if (changed_current_temperature && std::isnan(sensor->current_temperature) && std::isnan(current_temperature)) {
    changed_current_temperature = false;
  }
  if (changed_current_temperature) {
    ESP_LOGD(TAG, "changed current temperature from %f to %f", sensor->current_temperature, current_temperature);
    any_change |= true;
    sensor->current_temperature = current_temperature;
  }

  bool changed_mode = sensor->mode != mode;
  if (changed_mode) {
    ESP_LOGD(TAG, "changed mode from %i to %i", LOG_STR_ARG(climate::climate_mode_to_string(sensor->mode)),
             LOG_STR_ARG(climate::climate_mode_to_string(mode)));
    any_change |= true;
    sensor->mode = mode;
  }

  bool changed_target_temperature = sensor->target_temperature != target_temperature;
  if (changed_target_temperature && std::isnan(sensor->target_temperature) && std::isnan(target_temperature)) {
    changed_target_temperature = false;
  }
  if (changed_target_temperature) {
    ESP_LOGD(TAG, "changed target temperature from %f to %f", sensor->target_temperature, target_temperature);
    any_change |= true;
    sensor->target_temperature = target_temperature;
  }

  if (!any_change) {
    return;
  }

  sensor->publish_state();
};

}  // namespace balboa
}  // namespace esphome