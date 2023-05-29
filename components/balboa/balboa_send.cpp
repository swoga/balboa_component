#include "balboa.h"

namespace esphome {
namespace balboa {

void BalboaComponent::handle_msg_clear_to_send() {
  for (auto it = msg_send_bufer.cbegin(); it != msg_send_bufer.cend(); ++it) {
    auto msg = *it;
    // check if message should be sent in the future
    if (msg.time != 0 && msg.time > millis()) {
      continue;
    }

    ESP_LOGVV(TAG, "send message")
    send_direct(my_channel, msg.msg, msg.length);
    msg_send_bufer.erase(it);
    return;
  }

  send_nothing_to_send();
}

void BalboaComponent::send_nothing_to_send() {
  uint8_t msg[] = {MSG_NothingToSend};
  send_direct(my_channel, msg, 1);
}

void BalboaComponent::send_direct(uint8_t channel, uint8_t *msg, size_t msg_length) {
  // + channel, ?, ..msg.., CRC, ME
  size_t total_length = msg_length + 4;
  // + MS, length
  size_t buffer_length = total_length + 2;
  uint8_t buffer[buffer_length];

  buffer[0] = MSME;
  buffer[1] = total_length;
  buffer[2] = channel;
  buffer[3] = 0xBF;
  memcpy(buffer + 4, msg, msg_length);
  uint8_t crc = crc8(buffer + 1, buffer_length - 3);
  buffer[buffer_length - 2] = crc;
  buffer[buffer_length - 1] = MSME;

  ESP_LOGVV("tx: %s", format_hex_pretty(buffer, buffer_length))

  write_array(buffer, buffer_length);
}

void BalboaComponent::send_buffer(uint8_t msg[], size_t length, unsigned long time) {
  msg_send new_msg;
  new_msg.msg = msg;
  new_msg.length = length;
  new_msg.time = time;
  msg_send_bufer.push_back(new_msg);
}

void BalboaComponent::send_toggle_item(uint8_t item, unsigned long time) {
  uint8_t msg[] = {MSG_ToggleItem, item, 0x00};
  send_buffer(msg, 3, time);
}

}  // namespace balboa
}  // namespace esphome