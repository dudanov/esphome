#pragma once

#include "esphome/components/climate_ir/climate_ir.h"

namespace esphome {
namespace midea_ir {

class ControlData;
class SpecialData;

class MideaIR : public climate_ir::ClimateIR {
 public:
  MideaIR();
  /// Override control to change settings of the climate device.
  void control(const climate::ClimateCall &call) override;

  /// Set use of Fahrenheit units
  void set_use_fahrenheit(bool use_fahrenheit);

 protected:
  /// Transmit via IR the state of this climate controller.
  void transmit_state() override {}

  /// Handle received IR Buffer
  bool on_receive(remote_base::RemoteReceiveData src) override;
  ///
  void on_control_(const ControlData &data);
  ///
  void on_special_(uint8_t command);

  /// Transmit state via main frame
  void transmit_state_(uint32_t delay);
  /// Transmit special command
  void transmit_special_(uint32_t delay, uint8_t command);

  bool use_fahrenheit_;
};

}  // namespace midea_ir
}  // namespace esphome
