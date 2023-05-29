#include "select.h"

namespace esphome {
namespace balboa {

void BalboaLightSelect::control(const std::string &value) { light->turn_off(); };

}  // namespace balboa
}  // namespace esphome