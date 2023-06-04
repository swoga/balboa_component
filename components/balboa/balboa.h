#pragma once

#include <vector>
#include <cmath>
#include "esphome/core/version.h"
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/preferences.h"

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/globals/globals_component.h"

#include "esphome/components/time/real_time_clock.h"

#include "crc.h"
#include "switch.h"
#include "select.h"
#include "climate.h"

#define readBit(value, bit) (((value) >> (bit)) & 0x01)

namespace esphome {
namespace balboa {
class BalboaClimate;
class BalboaSwitch;
class BalboaLightSelect;

const size_t MAX_BUFFER_SIZE = 256;
const size_t MAX_MSG_SEND_BUFFER = 32;

const uint8_t MSME = 0x7E;
const uint8_t CHANNEL_MULTICAST = 0xFE;
const uint8_t CHANNEL_BROADCAST = 0xFF;

enum MessageType : uint8_t {
  NewClientClearToSend = 0x00,
  ChannelAssignmentRequest = 0x01,
  ChannelAssignmentResponse = 0x02,
  ChannelAssignmentAck = 0x03,
  ClearToSend = 0x06,
  NothingToSend = 0x07,
  ToggleItem = 0x11,
  StatusUpdate = 0x13,
  SetTargetTemp = 0x20,
  SetTime = 0x21,
};

const uint8_t ITEM_Restmode = 0x51;

static const char *const TAG = "balboa";

struct msg_send {
  MessageType type;
  std::vector<uint8_t> data;
  unsigned long time;
};

class BalboaComponent : public uart::UARTDevice, public Component {
 public:
  void setup() override;
  void loop() override;

  void set_id(std::string id) { this->id = id; }

  void set_circ_sensor(binary_sensor::BinarySensor *x) { circ = x; }
  void set_heating_sensor(binary_sensor::BinarySensor *x) { heating = x; }

  void set_highrange_switch(BalboaSwitch *x) { highrange = x; }
  void set_light1_switch(BalboaSwitch *x) { light1 = x; }
  void set_pump1_switch(BalboaSwitch *x) { pump1 = x; }
  void set_pump2_switch(BalboaSwitch *x) { pump2 = x; }

  void set_light1_color_select(BalboaLightSelect *x) { light1_color = x; }

  void set_thermostat(BalboaClimate *x) { thermostat = x; }

  void set_rtc(time::RealTimeClock *x) { rtc = x; }

  void set_serialenabled_var(globals::GlobalsComponent<bool> *x) { serialenabled = x; }

 protected:
  std::string id;

  binary_sensor::BinarySensor *circ;
  binary_sensor::BinarySensor *heating;

  BalboaSwitch *highrange;
  BalboaSwitch *light1;
  BalboaSwitch *pump1;
  BalboaSwitch *pump2;
  globals::GlobalsComponent<bool> *serialenabled;

  BalboaClimate *thermostat;

  BalboaLightSelect *light1_color;

  time::RealTimeClock *rtc;

  std::vector<uint8_t> buffer;

  unsigned long first_message = 0;
  unsigned long first_channel_message = 0;
  bool my_channel_confirmed = false;
  bool my_channel_requested = false;
  uint8_t my_channel = 0;
  ESPPreferenceObject my_channel_pref;
  void set_channel(uint8_t channel) {
    this->my_channel = channel;
    this->my_channel_pref.save(&channel);
    this->my_channel_confirmed = true;
  }

  std::vector<msg_send> tx_buffer;

  // receive
  void rs485_receive();
  bool parse();
  void handle_multicast(MessageType msg_type, uint8_t msg[], size_t length);
  void handle_unicast(MessageType msg_type, uint8_t msg[], size_t length);
  void handle_broadcast(MessageType msg_type, uint8_t msg[], size_t length);

  void handle_status_update(uint8_t msg[], size_t length);

  time_t last_time_sync = 0;
  void handle_time_sync(uint8_t hour, uint8_t minute);
  void send_set_time(uint8_t hour, uint8_t minute);

  // channel
  void handle_new_client_clear_to_send();
  void send_channel_request();
  void handle_channel_assignment_response(uint8_t msg[], size_t length);
  void send_channel_ack();
  void handle_unicast_unconfirmed(MessageType msg_type);

  // send
  void handle_msg_clear_to_send();
  void send_nothing_to_send();
  void send_direct(uint8_t channel, MessageType type, const std::vector<uint8_t> data);

  // publish
  void publish(binary_sensor::BinarySensor *sensor, bool value, bool change_only);
  void publish(BalboaSwitch *sensor, bool value, bool change_only);
  void publish(BalboaClimate *sensor, climate::ClimateMode mode, float current_temperature, float target_temperature,
               bool change_only);

 public:
  void send_buffer(MessageType type, const std::vector<uint8_t> data, unsigned long time = 0);
  void send_toggle_item(uint8_t item, unsigned long time = 0);
};

}  // namespace balboa
}  // namespace esphome