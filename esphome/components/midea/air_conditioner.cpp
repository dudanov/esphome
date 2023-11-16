#ifdef USE_ARDUINO

#include "esphome/core/log.h"
#include "air_conditioner.h"
#include "ac_adapter.h"

namespace esphome {
namespace midea {
namespace ac {

static void set_sensor(Sensor *sensor, float value) {
  if (sensor != nullptr && (!sensor->has_state() || sensor->get_raw_state() != value))
    sensor->publish_state(value);
}

template<typename T> void update_property(T &property, const T &value, bool &flag) {
  if (property != value) {
    property = value;
    flag = true;
  }
}

void AirConditioner::on_status_change() {
  bool need_publish = false;

  update_property(this->target_temperature, this->ac_.getTargetTemp(), need_publish);
  update_property(this->current_temperature, this->ac_.getIndoorTemp(), need_publish);
  auto mode = to_climate_mode(this->ac_.getMode());
  update_property(this->mode, mode, need_publish);
  auto swing_mode = to_climate_swing_mode(this->ac_.getSwingMode());
  update_property(this->swing_mode, swing_mode, need_publish);

  // Preset
  auto preset = this->ac_.getPreset();
  if (is_custom_midea_preset(preset)) {
    if (this->set_custom_preset_(to_custom_climate_preset(preset)))
      need_publish = true;
  } else if (this->set_preset_(to_climate_preset(preset))) {
    need_publish = true;
  }

  // Fan mode
  auto fan_mode = this->ac_.getFanMode();
  if (is_custom_midea_fan_mode(fan_mode)) {
    if (this->set_custom_fan_mode_(to_custom_climate_fan_mode(fan_mode)))
      need_publish = true;
  } else if (this->set_fan_mode_(to_climate_fan_mode(fan_mode))) {
    need_publish = true;
  }

  if (need_publish)
    this->publish_state();

  set_sensor(this->outdoor_sensor_, this->ac_.getOutdoorTemp());
  set_sensor(this->power_sensor_, this->ac_.getPowerUsage());
  set_sensor(this->humidity_sensor_, this->ac_.getIndoorHum());
}

void AirConditioner::control(const ClimateCall &call) {
  dudanov::midea::ac::Control ctrl{};
  if (call.get_target_temperature().has_value())
    ctrl.targetTemp = call.get_target_temperature().value();
  if (call.get_swing_mode().has_value())
    ctrl.swingMode = to_midea_swing_mode(call.get_swing_mode().value());
  if (call.get_mode().has_value())
    ctrl.mode = to_midea_mode(call.get_mode().value());
  if (call.get_preset().has_value()) {
    ctrl.preset = to_midea_preset(call.get_preset().value());
  } else if (call.get_custom_preset().has_value()) {
    ctrl.preset = to_midea_preset(call.get_custom_preset().value());
  }
  if (call.get_fan_mode().has_value()) {
    ctrl.fanMode = to_midea_fan_mode(call.get_fan_mode().value());
  } else if (call.get_custom_fan_mode().has_value()) {
    ctrl.fanMode = to_midea_fan_mode(call.get_custom_fan_mode().value());
  }
  this->ac_.control(ctrl);
}

ClimateTraits AirConditioner::traits() {
  auto traits = ClimateTraits();
  traits.set_supports_current_temperature(true);
  traits.set_visual_min_temperature(17);
  traits.set_visual_max_temperature(30);
  traits.set_visual_temperature_step(0.5);
  traits.set_supported_modes(this->supported_modes_);
  traits.set_supported_swing_modes(this->supported_swing_modes_);
  traits.set_supported_presets(this->supported_presets_);
  traits.set_supported_custom_presets(this->supported_custom_presets_);
  traits.set_supported_custom_fan_modes(this->supported_custom_fan_modes_);
  /* + MINIMAL SET OF CAPABILITIES */
  traits.add_supported_fan_mode(ClimateFanMode::CLIMATE_FAN_AUTO);
  traits.add_supported_fan_mode(ClimateFanMode::CLIMATE_FAN_LOW);
  traits.add_supported_fan_mode(ClimateFanMode::CLIMATE_FAN_MEDIUM);
  traits.add_supported_fan_mode(ClimateFanMode::CLIMATE_FAN_HIGH);
  if (this->ac_.getAutoconfStatus() == dudanov::midea::AUTOCONF_OK)
    to_climate_traits(traits, this->ac_.getCapabilities());
  if (!traits.get_supported_modes().empty())
    traits.add_supported_mode(ClimateMode::CLIMATE_MODE_OFF);
  if (!traits.get_supported_swing_modes().empty())
    traits.add_supported_swing_mode(ClimateSwingMode::CLIMATE_SWING_OFF);
  if (!traits.get_supported_presets().empty())
    traits.add_supported_preset(ClimatePreset::CLIMATE_PRESET_NONE);
  return traits;
}

void AirConditioner::dump_config() {
  ESP_LOGCONFIG(Constants::TAG, "MideaDongle:");
  ESP_LOGCONFIG(Constants::TAG, "  [x] Period: %dms", this->ac_.getPeriod());
  ESP_LOGCONFIG(Constants::TAG, "  [x] Response timeout: %dms", this->ac_.getTimeout());
  ESP_LOGCONFIG(Constants::TAG, "  [x] Request attempts: %d", this->ac_.getNumAttempts());
  if (this->ac_.getAutoconfStatus() == dudanov::midea::AUTOCONF_OK) {
    this->ac_.getCapabilities().dump();
  } else if (this->ac_.getAutoconfStatus() == dudanov::midea::AUTOCONF_ERROR) {
    ESP_LOGW(Constants::TAG,
             "Failed to get 0xB5 capabilities report. Suggest to disable it in config and manually set your "
             "appliance options.");
  }
  this->dump_traits_(Constants::TAG);
}

}  // namespace ac
}  // namespace midea
}  // namespace esphome

#endif  // USE_ARDUINO
