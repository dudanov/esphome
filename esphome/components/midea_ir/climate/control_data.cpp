#include "control_data.h"
#include "esphome/core/helpers.h"
#include "esphome/components/midea_ir/automations.h"

namespace esphome {
namespace midea_ir {

void ControlData::set_target_temperature(float temperature, bool use_fahrenheit) {
  long min = TEMPC_MIN, max = TEMPC_MAX;

  if (use_fahrenheit) {
    min = TEMPF_MIN, max = TEMPF_MAX;
    temperature = celsius_to_fahrenheit(temperature);
  }

  const auto temp = clamp(lroundf(temperature), min, max);
  this->set_value_(2, temp - min, 31);
  this->set_fahrenheit(use_fahrenheit);
}

float ControlData::get_target_temperature() const {
  const uint8_t temp = this->get_value_(2, 31);

  if (this->get_fahrenheit())
    return round_half(fahrenheit_to_celsius(temp + TEMPF_MIN));

  return temp + TEMPC_MIN;
}

void ControlData::fix() {
  // In FAN_AUTO, modes COOL, HEAT and FAN_ONLY bit #5 in byte #1 must be set
  const uint8_t value = this->get_value_(1, 31);

  if (value == 0 || value == 3 || value == 4)
    this->set_mask_(1, true, 32);

  // In FAN_ONLY mode we need to set all temperature bits
  if (this->get_mode_() == MODE_FAN_ONLY)
    this->set_mask_(2, true, 31);
}

void ControlData::set_mode(ClimateMode mode) {
  switch (mode) {
    case ClimateMode::CLIMATE_MODE_OFF:
      this->set_power_(false);
      return;
    case ClimateMode::CLIMATE_MODE_COOL:
      this->set_mode_(MODE_COOL);
      break;
    case ClimateMode::CLIMATE_MODE_DRY:
      this->set_mode_(MODE_DRY);
      break;
    case ClimateMode::CLIMATE_MODE_FAN_ONLY:
      this->set_mode_(MODE_FAN_ONLY);
      break;
    case ClimateMode::CLIMATE_MODE_HEAT:
      this->set_mode_(MODE_HEAT);
      break;
    default:
      this->set_mode_(MODE_AUTO);
      break;
  }
  this->set_power_(true);
}

ClimateMode ControlData::get_mode() const {
  if (!this->get_power_())
    return ClimateMode::CLIMATE_MODE_OFF;
  switch (this->get_mode_()) {
    case MODE_COOL:
      return ClimateMode::CLIMATE_MODE_COOL;
    case MODE_DRY:
      return ClimateMode::CLIMATE_MODE_DRY;
    case MODE_FAN_ONLY:
      return ClimateMode::CLIMATE_MODE_FAN_ONLY;
    case MODE_HEAT:
      return ClimateMode::CLIMATE_MODE_HEAT;
    default:
      return ClimateMode::CLIMATE_MODE_HEAT_COOL;
  }
}

void ControlData::set_fan_mode(ClimateFanMode mode) {
  switch (mode) {
    case ClimateFanMode::CLIMATE_FAN_LOW:
      this->set_fan_mode_(FAN_LOW);
      break;
    case ClimateFanMode::CLIMATE_FAN_MEDIUM:
      this->set_fan_mode_(FAN_MEDIUM);
      break;
    case ClimateFanMode::CLIMATE_FAN_HIGH:
      this->set_fan_mode_(FAN_HIGH);
      break;
    default:
      this->set_fan_mode_(FAN_AUTO);
      break;
  }
}

ClimateFanMode ControlData::get_fan_mode() const {
  switch (this->get_fan_mode_()) {
    case FAN_LOW:
      return ClimateFanMode::CLIMATE_FAN_LOW;
    case FAN_MEDIUM:
      return ClimateFanMode::CLIMATE_FAN_MEDIUM;
    case FAN_HIGH:
      return ClimateFanMode::CLIMATE_FAN_HIGH;
    default:
      return ClimateFanMode::CLIMATE_FAN_AUTO;
  }
}

}  // namespace midea_ir
}  // namespace esphome
