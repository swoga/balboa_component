#include "balboa.h"

namespace esphome {
namespace balboa {

void BalboaComponent::setup() {
  uint32_t hash = fnv1_hash("balboa_channel_" + this->id);
  this->my_channel_pref = global_preferences->make_preference<uint8_t>(hash);
  this->my_channel_pref.load(&this->my_channel);
}

void BalboaComponent::loop() {
  int available = this->available();
  if (available == 0) {
    return;
  }

  this->rs485_receive();
}

}  // namespace balboa
}  // namespace esphome