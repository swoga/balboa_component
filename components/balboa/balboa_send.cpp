#include "balboa.h"

namespace esphome {
namespace balboa {

void BalboaComponent::handle_msg_clear_to_send() {
  for (auto it = tx_buffer.cbegin(); it != tx_buffer.cend(); ++it) {
    auto msg = *it;
    // check if message should be sent in the future
    if (msg.time != 0 && msg.time > millis()) {
      continue;
    }

    ESP_LOGV(TAG, "got clear to send, send msg from buffer");
    send_direct(my_channel, msg.type, msg.data);
    tx_buffer.erase(it);
    return;
  }

  send_nothing_to_send();
}

void BalboaComponent::send_nothing_to_send() {
  ESP_LOGVV(TAG, "nothing to send");
  send_direct(my_channel, MessageType::NothingToSend, {});
}

void BalboaComponent::send_direct(uint8_t channel, MessageType type, const std::vector<uint8_t> data) {
  // + channel, ?, type, ..data.., CRC, ME
  size_t msg_length = data.size() + 5;
  // + MS, length
  size_t frame_length = msg_length + 2;
  std::vector<uint8_t> frame;
  frame.reserve(frame_length);

  frame.push_back(MSME);
  frame.push_back((uint8_t) msg_length);
  frame.push_back(channel);
  frame.push_back(0xBF);
  frame.push_back(type);
  frame.insert(frame.end(), data.begin(), data.end());
  uint8_t crc = crc8(&frame[1], frame_length - 3);
  frame.push_back(crc);
  frame.push_back(MSME);

  ESP_LOGVV(TAG, "tx: %s", format_hex_pretty(frame).c_str());

  write_array(frame);
}

void BalboaComponent::send_buffer(MessageType type, const std::vector<uint8_t> data, unsigned long time) {
  ESP_LOGV(TAG, "add msg of type %02X with data %s to buffer to send at %i", type, format_hex_pretty(data).c_str(),
           time);
  if (tx_buffer.size() > MAX_MSG_SEND_BUFFER) {
    ESP_LOGW(TAG, "msg send buffer overflow");
    tx_buffer.erase(tx_buffer.begin());
  }
  tx_buffer.push_back({type, data, time});
}

void BalboaComponent::send_toggle_item(uint8_t item, unsigned long time) {
  ESP_LOGV(TAG, "toggle item %02X", item);
  send_buffer(MessageType::ToggleItem, {item, 0x00}, time);
}

}  // namespace balboa
}  // namespace esphome