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

    ESP_LOGV(TAG, "got clear to send, send msg from buffer");
    send_direct(my_channel, &msg.msg[0], msg.msg.size());
    msg_send_bufer.erase(it);
    return;
  }

  send_nothing_to_send();
}

void BalboaComponent::send_nothing_to_send() {
  ESP_LOGVV(TAG, "nothing to send");
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

  ESP_LOGVV(TAG, "tx: %s", format_hex_pretty(buffer, buffer_length).c_str());

  write_array(buffer, buffer_length);
}

void BalboaComponent::send_buffer(uint8_t msg[], size_t length, unsigned long time) {
  ESP_LOGVV(TAG, "add msg (%s) to buffer to send at %i", format_hex_pretty(msg, length).c_str(), time);
  std::vector<uint8_t> msg_vec(&msg[0], &msg[length]);
  msg_send new_msg;
  new_msg.msg = msg_vec;
  new_msg.time = time;
  if (msg_send_bufer.size() > MAX_MSG_SEND_BUFFER) {
    ESP_LOGW(TAG, "msg send buffer overflow");
    msg_send_bufer.erase(msg_send_bufer.begin());
  }
  msg_send_bufer.push_back(new_msg);
}

void BalboaComponent::send_toggle_item(uint8_t item, unsigned long time) {
  ESP_LOGVV(TAG, "toggle item %i", item);
  uint8_t msg[] = {MSG_ToggleItem, item, 0x00};
  send_buffer(msg, 3, time);
}

}  // namespace balboa
}  // namespace esphome