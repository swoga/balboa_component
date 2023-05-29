#include "balboa.h"

namespace esphome {
namespace balboa {

void BalboaComponent::rs485_receive() {
  uint8_t data;
  while (this->read_byte(&data)) {
    this->buffer.push_back(data);
    this->parse();
  }
}

void pop_n(std::deque<uint8_t> &buffer, uint8_t n) {
  if (n >= buffer.size()) {
    buffer.clear();
    return;
  }

  auto it = buffer.begin();
  it += n;
  buffer.erase(buffer.begin(), it);
}

void BalboaComponent::parse() {
  while (this->buffer.size() >= 2) {
    uint8_t value = this->buffer[0];
    if (value != MSME) {
      // discard non-MS byte
      pop_n(this->buffer, 1);
      continue;
    }

    // MS found

    uint8_t length = this->buffer[1];
    // check if length is plausible
    if (length < 5 || length >= 127) {
      // discard MS and length
      pop_n(this->buffer, 2);
      return;
    }

    uint8_t totalLength = length + 2;
    if (totalLength > this->buffer.size()) {
      // wait for more bytes
      return;
    }

    uint8_t expectME = this->buffer[totalLength - 1];
    if (expectME != MSME) {
      // discard MS and try again
      pop_n(this->buffer, 1);
      continue;
    }

    // ME found

    // buffer + 1 -> start after MS, at length
    // totalLength - 3 -> remove MS and cut off CRC, ME
    uint8_t hasCRC = crc8(&this->buffer[1], totalLength - 3);
    uint8_t expectCRC = this->buffer[totalLength - 2];
    if (expectCRC != hasCRC) {
      // DEBUG_SPA("invalid CRC, has: %02X, exp: %02X", hasCRC, expectCRC);
      pop_n(this->buffer, totalLength);
      return;
    }

    // start after MS and length, at channel
    uint8_t *msgStart = &this->buffer[2];
    // remove MS, length and cut off CRC, ME (min = 3)
    uint8_t msgLength = totalLength - 4;

    // message received

    if (this->first_message == 0) {
      this->first_message = millis();
    }

    if (this->my_channel != 0 && !this->my_channel_confirmed) {
      unsigned long elapsed = millis() - first_message;
      if (elapsed > 10000) {
        // no messages until timeout, remove cached channel
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
  }
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
  // check if esphome has synced time
  if (!now.is_valid()) {
    return;
  }

  // only update time once an hour
  if (now.timestamp - last_time_sync < 3600) {
    return;
  }
  last_time_sync = now.timestamp;

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

  // ignore diff smaller than 5 minutes
  if (diff < 5) {
    return;
  }

  ESP_LOGI(TAG, "time diff %i minutes", diff);
  send_set_time(now.hour, now.minute);
}

void BalboaComponent::send_set_time(uint8_t hour, uint8_t minute) {
  ESP_LOGI(TAG, "send set time %i:%i", hour, minute);
  uint8_t msg[] = {MSG_SetTime, hour, minute};
  send_buffer(msg, 3);
}

}  // namespace balboa
}  // namespace esphome