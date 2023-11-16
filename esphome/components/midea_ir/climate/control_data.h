#pragma once

#include "esphome/components/climate/climate_mode.h"
#include "esphome/components/remote_base/midea_protocol.h"

namespace esphome {
namespace midea_ir {

using climate::ClimateMode;
using climate::ClimateFanMode;
using remote_base::MideaData;

constexpr uint8_t TEMPC_MIN = 17;
constexpr uint8_t TEMPC_MAX = 30;
constexpr uint8_t TEMPF_MIN = 62;
constexpr uint8_t TEMPF_MAX = 86;

class ControlData : public MideaData {
 public:
  // Default constructor (power: ON, mode: AUTO, fan: AUTO, temp: 25C)
  ControlData() : MideaData({MIDEA_TYPE_CONTROL, 0x82, 0x48, 0xFF, 0xFF}) {}
  // Copy from Base
  ControlData(const MideaData &data) : MideaData(data) {}

  void set_target_temperature(float temp, bool use_fahrenheit);
  float get_target_temperature() const;

  void set_mode(ClimateMode mode);
  ClimateMode get_mode() const;
  bool has_mode(ClimateMode mode) const { return this->get_mode() == mode; }

  void set_fan_mode(ClimateFanMode mode);
  ClimateFanMode get_fan_mode() const;

  void set_sleep_preset(bool value) { this->set_mask_(1, value, 64); }
  bool get_sleep_preset() const { return this->get_value_(1, 64); }

  void set_fahrenheit(bool value) { this->set_mask_(2, value, 32); }
  bool get_fahrenheit() const { return this->get_value_(2, 32); }

  void fix();

 protected:
  enum Mode : uint8_t {
    MODE_COOL,
    MODE_DRY,
    MODE_AUTO,
    MODE_HEAT,
    MODE_FAN_ONLY,
  };
  enum FanMode : uint8_t {
    FAN_AUTO,
    FAN_LOW,
    FAN_MEDIUM,
    FAN_HIGH,
  };
  void set_fan_mode_(FanMode mode) { this->set_value_(1, mode, 3, 3); }
  FanMode get_fan_mode_() const { return static_cast<FanMode>(this->get_value_(1, 3, 3)); }
  void set_mode_(Mode mode) { this->set_value_(1, mode, 7); }
  Mode get_mode_() const { return static_cast<Mode>(this->get_value_(1, 7)); }
  void set_power_(bool value) { this->set_mask_(1, value, 128); }
  bool get_power_() const { return this->get_value_(1, 128); }
};

}  // namespace midea_ir
}  // namespace esphome
