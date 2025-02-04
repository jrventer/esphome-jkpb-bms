#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/hal.h"
#include "binary_output.h"


namespace esphome {


namespace jk_rs485_sniffer {

enum ProtocolVersion {
  PROTOCOL_VERSION_JK04,
  PROTOCOL_VERSION_JK02_24S,
  PROTOCOL_VERSION_JK02_32S,
};

class JkRS485SnifferDevice;

class JkRS485Sniffer : public uart::UARTDevice, public output::BinaryOutput, public Component {
 public:
  JkRS485Sniffer() = default;


  void set_talk_pin(GPIOPin *pin) { talk_pin_ = pin; }

  void setup() override {
    this->turn_off();
    //this->talk_pin_->pin_mode(esphome::gpio::FLAG_OUTPUT);
    this->talk_pin_->setup();
    //this->turn_on();
    this->talk_pin_->digital_write(0); 
  //
    for (uint8_t cont=0;cont<16;cont++){
        rs485_network_node[cont].available=0;
        rs485_network_node[cont].last_message_received=0;
        rs485_network_node[cont].last_device_settings_request_sent=0;
        rs485_network_node[cont].last_cell_info_request_sent=0;
        rs485_network_node[cont].last_device_info_request_sent=0;
        rs485_network_node[cont].counter_cell_info_received=0;
        rs485_network_node[cont].counter_device_settings_received=0;
        rs485_network_node[cont].counter_device_info_received=0;        
    }    
    last_master_activity=0;
    last_message_sent_acting_as_master=0;
    last_network_scan=0;

    act_as_master=false;

    pooling_index.node_address=0x0f;
    pooling_index.frame_type=2;
    pooling_index.scan_address=0x01;

    nodes_available_number=0;
    nodes_available.reserve(17); 
    for (uint8_t cont = 0; cont < 16; cont++) {
        nodes_available.push_back('0');
    }
    nodes_available.push_back('\0');
  }

  void loop() override;

  void dump_config() override;

  void register_device(JkRS485SnifferDevice *device) { this->devices_.push_back(device); }

  float get_setup_priority() const override;

  void set_rx_timeout(uint16_t rx_timeout) { rx_timeout_ = rx_timeout; }


 protected:
  ProtocolVersion protocol_version_{PROTOCOL_VERSION_JK02_32S};
  
  bool act_as_master;
  uint32_t last_master_activity;
  uint32_t last_message_sent_acting_as_master;
  uint32_t last_network_scan;


  void printBuffer(uint16_t max_length);
  uint8_t manage_rx_buffer_(void);
  void set_node_availability(uint8_t address,bool value);
  std::string nodes_available_to_string();

  void detected_master_activity_now(void);
  void send_request_to_slave(uint8_t node_address, uint8_t frame_type);
  bool calculate_next_pooling(void);

  std::vector<uint8_t> rx_buffer_;
  uint16_t rx_timeout_{50};
  uint32_t last_jk_rs485_network_activity_{0};
  std::vector<JkRS485SnifferDevice *> devices_;  

  void write_state(bool state) override { this->talk_pin_->digital_write(state); }
  //void write_state(bool state) override { this->set_state(state); }
  GPIOPin *talk_pin_;

  struct struct_rs485_network_node {
     bool available;
     uint32_t last_message_received;
     uint32_t last_device_settings_request_sent;
     uint32_t last_device_info_request_sent;
     uint32_t last_cell_info_request_sent;
     uint16_t counter_cell_info_received;
     uint16_t counter_device_settings_received;
     uint16_t counter_device_info_received;      
  };
  struct struct_rs485_network_node rs485_network_node[16];
  
  std::string nodes_available;
  uint8_t nodes_available_number;

  struct struct_pooling_index {
     uint8_t node_address;
     uint8_t frame_type;
     uint8_t scan_address;
  };

  struct struct_pooling_index pooling_index;
};

class JkRS485SnifferDevice {
 public:
  void set_parent(JkRS485Sniffer *parent) { parent_ = parent; }
//  void set_address(uint8_t address) { address_ = address; }
  virtual void on_jk_rs485_sniffer_data(const uint8_t &origin_address, const uint8_t &frame_type, const std::vector<uint8_t> &data, const std::string &nodes_available) = 0;

 protected:
  friend JkRS485Sniffer;

  JkRS485Sniffer *parent_;
  //uint8_t address_;
};

}  // namespace jk_rs485_sniffer
}  // namespace esphome
