#include "balboa.h"

namespace esphome {
namespace balboa {

const uint8_t CHANNEL_REQUEST_HASH1 = 0xF1;
const uint8_t CHANNEL_REQUEST_HASH2 = 0x73;

void BalboaComponent::handle_new_client_clear_to_send() {
  // ignore if we already have a channel
  if (this->my_channel != 0) {
    return;
  }
  this->send_channel_request();
}

void BalboaComponent::send_channel_request() {
  if (my_channel_requested) {
    ESP_LOGVV(TAG, "channel already requested")
    return;
  }
  ESP_LOGD(TAG, "send channel request");
  my_channel_requested = true;

  uint8_t msg[] = {MSG_ChannelAssignmentRequest, 0x02, CHANNEL_REQUEST_HASH1, CHANNEL_REQUEST_HASH2};
  send_direct(CHANNEL_MULTICAST, msg, 4);
}

void BalboaComponent::handle_channel_assignment_response(uint8_t msg[], size_t length) {
  // ignore if we already have a channel
  if (this->my_channel != 0) {
    return;
  }
  if (length < 3) {
    return;
  }
  this->set_channel(msg[0]);

  // ignore responses that are not ours
  if (msg[1] != CHANNEL_REQUEST_HASH1) {
    return;
  }
  if (msg[2] != CHANNEL_REQUEST_HASH2) {
    return;
  }

  this->send_channel_ack();
}

void BalboaComponent::send_channel_ack() {
  ESP_LOGI(TAG, "send channel ack: %02X", my_channel);
  uint8_t msg[] = {MSG_ChannelAssignmentAck};
  send_direct(my_channel, msg, 1);
}

void BalboaComponent::handle_unicast_unconfirmed(uint8_t msg_type) {
  bool is_clear_to_send = msg_type == MSG_ClearToSend;
  if (!is_clear_to_send) {
    ESP_LOGI(TAG, "channel lost, received client msg");
    this->set_channel(0);
    return;
  }

  if (first_channel_message == 0) {
    ESP_LOGD(TAG, "received clear to send on unconfirmed channel");
    first_channel_message = millis();
    return;
  }

  unsigned long elapsed = millis() - first_channel_message;
  if (elapsed > 5000) {
    ESP_LOGI(TAG, "confirm channel, no other messages received");
    this->my_channel_confirmed = true;
  }
}

}  // namespace balboa
}  // namespace esphome