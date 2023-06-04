#include "select.h"

namespace esphome {
namespace balboa {

void BalboaLightSelect::control(const std::string &value) {
  light->turn_off();
  auto index = index_of(value);
  if (!index.has_value()) {
    return;
  }
  auto delay = 2000;
  auto toggles = index.value() * 2 + 1;
  for (auto i = 0; i < toggles; i++) {
    light->get_parent()->set_timeout(delay, [this]() { this->light->send_toggle_item(); });
    delay += 200;
  }
};

}  // namespace balboa
}  // namespace esphome