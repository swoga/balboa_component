#include "balboa.h"

namespace esphome {
namespace balboa {

void BalboaComponent::rs485_receive() {
  if (serialenabled && !serialenabled->value()) {
    return;
  }

  while (available() > 0) {
    uint8_t data;
    if (!read_byte(&data)) {
      continue;
    }
    if (buffer.size() > MAX_BUFFER_SIZE) {
      ESP_LOGW(TAG, "receive buffer overflow");
      buffer.erase(buffer.begin());
    }
    buffer.push_back(data);
  }
  while (buffer.size() >= 2) {
    auto cont = parse();
    if (!cont) {
      break;
    }
  }
}

void pop_n(std::vector<uint8_t> &buffer, uint8_t n) {
  if (n >= buffer.size()) {
    buffer.clear();
    return;
  }

  auto it = buffer.begin();
  it += n;
  buffer.erase(buffer.begin(), it);
}

bool BalboaComponent::parse() {
  uint8_t value = this->buffer[0];
  if (value != MSME) {
    ESP_LOGV(TAG, "discard non-MS byte %02X", value);
    pop_n(this->buffer, 1);
    return true;
  }

  // MS found

  uint8_t length = this->buffer[1];
  if (length < 5 || length >= 127) {
    ESP_LOGV(TAG, "implausible length of %i, discard MS and length", length);
    pop_n(this->buffer, 2);
    return true;
  }

  uint8_t totalLength = length + 2;
  if (totalLength > this->buffer.size()) {
    ESP_LOGVV(TAG, "received %i bytes, wait for a total of %i", buffer.size(), totalLength);
    return false;
  }

  uint8_t expectME = this->buffer[totalLength - 1];
  if (expectME != MSME) {
    // discard MS and try again
    ESP_LOGV(TAG, "unexpected end %02X, discard MS and try again", expectME);
    pop_n(this->buffer, 1);
    return true;
  }

  ESP_LOGVV(TAG, "received frame: %s", format_hex_pretty(&buffer[0], totalLength).c_str());

  // ME found

  // buffer + 1 -> start after MS, at length
  // totalLength - 3 -> remove MS and cut off CRC, ME
  uint8_t hasCRC = crc8(&this->buffer[1], totalLength - 3);
  uint8_t expectCRC = this->buffer[totalLength - 2];
  if (expectCRC != hasCRC) {
    ESP_LOGV(TAG, "invalid crc %02X, expected %02X, discard whole message", hasCRC, expectCRC);
    pop_n(this->buffer, totalLength);
    return true;
  }

  // start after MS and length, at channel
  uint8_t *msgStart = &this->buffer[2];
  // remove MS, length and cut off CRC, ME (min = 3)
  uint8_t msgLength = totalLength - 4;

  // message received

  if (this->first_message == 0) {
    ESP_LOGD(TAG, "first message received");
    this->first_message = millis();
  }

  if (this->my_channel != 0 && !this->my_channel_confirmed) {
    unsigned long elapsed = millis() - first_message;
    if (elapsed > 10000) {
      ESP_LOGI(TAG, "no messages received on cached channel until timeout, reset channel");
      this->set_channel(0);
    }
  }

  uint8_t channel = msgStart[0];
  uint8_t msg_type = msgStart[2];

  // remove channel, unknown byte (0xAF / 0xBF) and msg type
  msgStart += 3;
  // (min = 0)
  msgLength -= 3;

  bool msg_has_channel = channel != 0;

  if (channel == CHANNEL_MULTICAST) {
    this->handle_multicast(msg_type, msgStart, msgLength);
  } else if (channel == CHANNEL_BROADCAST) {
    this->handle_broadcast(msg_type, msgStart, msgLength);
  } else if (msg_has_channel && channel == my_channel) {
    if (my_channel_confirmed) {
      this->handle_unicast(msg_type, msgStart, msgLength);
    } else {
      // received msg on cached channel, need to validate if still ours
      this->handle_unicast_unconfirmed(msg_type);
    }
  }

  pop_n(this->buffer, totalLength);

  return true;
}

void BalboaComponent::handle_multicast(uint8_t type, uint8_t msg[], size_t length) {
  switch (type) {
    case MSG_NewClientClearToSend:
      handle_new_client_clear_to_send();
      break;
    case MSG_ChannelAssignmentResponse:
      handle_channel_assignment_response(msg, length);
      break;
  }
}

void BalboaComponent::handle_unicast(uint8_t type, uint8_t msg[], size_t length) {
  switch (type) {
    case MSG_ClearToSend:
      handle_msg_clear_to_send();
      break;
  }
}

void BalboaComponent::handle_broadcast(uint8_t type, uint8_t msg[], size_t length) {
  switch (type) {
    case MSG_StatusUpdate:
      handle_status_update(msg, length);
      break;
  }
}

float get_temp_value(uint8_t value) {
  // hardcoded to celcius
  if (1 == 1) {
    return value / 2.0;
  } else {
    return value;
  }
}

void BalboaComponent::handle_status_update(uint8_t msg[], int length) {
  if (length < 27) {
    ESP_LOGV(TAG, "discard status update");
    return;
  }

  publish(highrange, readBit(msg[10], 2), true);
  publish(heating, readBit(msg[10], 4) || readBit(msg[10], 5), true);
  publish(pump1, msg[11] & 0x03, true);
  publish(pump2, (msg[11] >> 2) & 0x03, true);
  publish(circ, readBit(msg[13], 1), true);
  publish(light1, readBit(msg[14], 0), true);

  // restmode
  auto mode = msg[5] == 1 ? climate::CLIMATE_MODE_OFF : climate::CLIMATE_MODE_HEAT;
  auto target_temperature = get_temp_value(msg[20]);
  // only use value, if temp is reported
  float current_temperature = NAN;
  if (msg[2] < 127) {
    current_temperature = get_temp_value(msg[2]);
  }
  publish(thermostat, mode, current_temperature, target_temperature, true);

  handle_time_sync(msg[3], msg[4]);
}

void BalboaComponent::handle_time_sync(uint8_t hour, uint8_t minute) {
  auto now = rtc->now();

  if (!now.is_valid()) {
    ESP_LOGV(TAG, "MCU clock not synced, skip");
    return;
  }

  // only update time once an hour
  if (now.timestamp - last_time_sync < 3600) {
    return;
  }
  last_time_sync = now.timestamp;

  ESP_LOGD(TAG, "check clock drift");

  int cur = now.hour * 60 + now.minute;
  int has = hour * 60 + minute;

  // shortest distance to/from midnight
  int cur_tofrom_mid = std::min(abs(24 * 60 - cur), cur);
  int has_tofrom_mid = std::min(abs(24 * 60 - has), has);
  // if times crossing midnight, this is the difference
  int diff_over_mid = cur_tofrom_mid + has_tofrom_mid;
  int diff = 0;
  if (cur >= has) {
    diff = cur - has;
  } else {
    diff = has - cur;
  }
  diff = std::min(diff, diff_over_mid);

  ESP_LOGD(TAG, "time diff %i minutes", diff);

  // ignore diff smaller than 5 minutes
  if (diff < 5) {
    return;
  }

  send_set_time(now.hour, now.minute);
}

void BalboaComponent::send_set_time(uint8_t hour, uint8_t minute) {
  ESP_LOGI(TAG, "send set time %i:%i", hour, minute);
  uint8_t msg[] = {MSG_SetTime, hour, minute};
  send_buffer(msg, 3);
}

}  // namespace balboa
}  // namespace esphome