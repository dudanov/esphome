#pragma once

#include "esphome/components/midea_ir/automations.h"
#include "esphome/components/button/button.h"

namespace esphome {
namespace midea_ir {

class SpecialButton : public button::Button, public remote_base::RemoteTransmittable {
 public:
  void press_action() override { SpecialData::transmit(this->transmitter_, this->command_); }
  void set_command(uint8_t command) { this->command_ = command; }

 protected:
  uint8_t command_;
};

}  // namespace midea_ir
}  // namespace esphome
