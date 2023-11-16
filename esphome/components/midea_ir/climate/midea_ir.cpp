#include "midea_ir.h"
#include "control_data.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/components/coolix/coolix.h"
#include "esphome/components/climate_ir/control_helper.h"
#include "esphome/components/midea_ir/automations.h"

namespace esphome {
namespace midea_ir {

using climate_ir::ControlHelper;

static const char *const TAG = "midea_ir.climate";

MideaIR::MideaIR()
    : ClimateIR(
          TEMPC_MIN, TEMPC_MAX, 1.0f, true, true,
          {climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW, climate::CLIMATE_FAN_MEDIUM, climate::CLIMATE_FAN_HIGH},
          {climate::CLIMATE_SWING_OFF, climate::CLIMATE_SWING_VERTICAL},
          {climate::CLIMATE_PRESET_NONE, climate::CLIMATE_PRESET_SLEEP, climate::CLIMATE_PRESET_BOOST}) {}

void MideaIR::set_use_fahrenheit(bool use_fahrenheit) {
  this->temperature_step_ = use_fahrenheit ? 0.5f : 1.0f;
  this->use_fahrenheit_ = use_fahrenheit;
}

bool has_uncommitted() { return false; }
template<typename T, typename M, typename... Ts> bool has_uncommitted(const ControlHelper<T, M> &helper, Ts... x) {
  return !helper.is_committed() || has_uncommitted(x...);
}

static void mcontrol(climate::Climate *climate, const climate::ClimateCall &call) {
  auto mode = ControlHelper(climate->mode, call.get_mode());
  auto temp = ControlHelper(climate->target_temperature, call.get_target_temperature());
  auto fan = ControlHelper(climate->fan_mode, call.get_fan_mode(), climate::CLIMATE_FAN_AUTO);
  auto preset = ControlHelper(climate->preset, call.get_preset(), climate::CLIMATE_PRESET_NONE);
  auto swing = ControlHelper(climate->swing_mode, call.get_swing_mode());

  switch (mode.get_value()) {
    case climate::CLIMATE_MODE_HEAT_COOL:
      fan.commit(climate::CLIMATE_FAN_AUTO);
      if (preset.has_value(climate::CLIMATE_PRESET_BOOST))
        preset.commit(climate::CLIMATE_PRESET_NONE);
      break;
    case climate::CLIMATE_MODE_DRY:
      fan.commit(climate::CLIMATE_FAN_AUTO);
      /* FALLTHROUGH */
    case climate::CLIMATE_MODE_OFF:
    case climate::CLIMATE_MODE_FAN_ONLY:
      temp.commit();
      preset.commit(climate::CLIMATE_PRESET_NONE);
    default:
      break;
  }

  if (!preset.has_value(climate::CLIMATE_PRESET_NONE))
    fan.commit(climate::CLIMATE_FAN_AUTO);

  // Turn off BOOST command is required ONLY without changing the current mode
  const bool boost_off =
      mode.is_committed() && preset.is_from_to(climate::CLIMATE_PRESET_BOOST, climate::CLIMATE_PRESET_NONE);
}

void MideaIR::control(const climate::ClimateCall &call) {
  auto new_mode = call.get_mode().value_or(this->mode);
  auto new_temp = call.get_target_temperature().value_or(this->target_temperature);
  auto new_fan = call.get_fan_mode().value_or(this->fan_mode.value_or(climate::CLIMATE_FAN_AUTO));
  auto new_preset = call.get_preset().value_or(this->preset.value_or(climate::CLIMATE_PRESET_NONE));
  auto new_swing = call.get_swing_mode().value_or(this->swing_mode);

  // Modes constraints
  switch (new_mode) {
    case climate::CLIMATE_MODE_HEAT_COOL:
      this->fan_mode = new_fan = climate::CLIMATE_FAN_AUTO;
      if (new_preset == climate::CLIMATE_PRESET_BOOST)
        this->preset = new_preset = climate::CLIMATE_PRESET_NONE;
      break;
    case climate::CLIMATE_MODE_DRY:
      this->fan_mode = new_fan = climate::CLIMATE_FAN_AUTO;
      /* FALLTHROUGH */
    case climate::CLIMATE_MODE_OFF:
    case climate::CLIMATE_MODE_FAN_ONLY:
      this->preset = new_preset = climate::CLIMATE_PRESET_NONE;
      this->target_temperature = new_temp;
    default:
      break;
  }

  // Presets constraints
  if (new_preset != climate::CLIMATE_PRESET_NONE)
    this->fan_mode = new_fan = climate::CLIMATE_FAN_AUTO;

  // Turn off BOOST command is required ONLY without changing the current mode
  const bool boost_off = new_mode == this->mode && new_preset == climate::CLIMATE_PRESET_NONE &&
                         this->preset.value_or(climate::CLIMATE_PRESET_NONE) == climate::CLIMATE_PRESET_BOOST;

  uint32_t timeout = 0;

  if (boost_off) {
    ESP_LOGD(TAG, "SENDING BOOST OFF. TIMEOUT: %dms", timeout);
    this->transmit_special_(0, TURBO_TOGGLE);
    this->preset = climate::CLIMATE_PRESET_NONE;
    timeout += 500;
  }

  bool main_frame = false;

  if (new_mode != this->mode) {
    main_frame = true;
    this->preset = climate::CLIMATE_PRESET_NONE;
  } else if (new_temp != this->target_temperature)
    main_frame = true;
  else if (new_fan != this->fan_mode)
    main_frame = true;
  else if (new_preset != this->preset && new_preset != climate::CLIMATE_PRESET_BOOST)
    main_frame = true;

  // BOOST command to disable preset is required ONLY without changing the current mode
  bool boost_on = new_preset == climate::CLIMATE_PRESET_BOOST && this->preset != climate::CLIMATE_PRESET_BOOST;

  this->mode = new_mode;
  this->swing_mode = new_swing;
  this->fan_mode = new_fan;
  this->preset = new_preset;
  this->target_temperature = new_temp;

  if (main_frame) {
    ESP_LOGD(TAG, "SENDING MAIN FRAME. TIMEOUT: %dms", timeout);
    this->transmit_state_(timeout);
    timeout += 500;
  }

  if (boost_on) {
    ESP_LOGD(TAG, "SENDING BOOST ON. TIMEOUT: %dms", timeout);
    this->transmit_special_(timeout, TURBO_TOGGLE);
    timeout += 500;
  }

  this->publish_state();
}

void MideaIR::transmit_state_(uint32_t delay) {
  this->set_timeout(delay, [this]() {
    ControlData frame;
    frame.set_target_temperature(this->target_temperature, this->use_fahrenheit_);
    frame.set_mode(this->mode);
    frame.set_fan_mode(this->fan_mode.value_or(climate::CLIMATE_FAN_AUTO));
    frame.set_sleep_preset(this->preset == climate::CLIMATE_PRESET_SLEEP);
    frame.fix();
    frame.transmit_cs(this->transmitter_);
  });
}

void MideaIR::transmit_special_(uint32_t delay, uint8_t command) {
  this->set_timeout(delay, [this, command]() { SpecialData::transmit(this->transmitter_, command); });
}

bool MideaIR::on_receive(remote_base::RemoteReceiveData src) {
  auto decoded = remote_base::MideaProtocol().decode(src);

  if (!decoded.has_value())
    return coolix::CoolixClimate::on_coolix(this, src);

  const auto &data = decoded.value();

  ESP_LOGD(TAG, "Decoded Midea IR data: %s", data.to_string().c_str());

  switch (data.type()) {
    case MideaData::MIDEA_TYPE_CONTROL:
      this->on_control_(data.to<ControlData>());
      break;
    case MideaData::MIDEA_TYPE_SPECIAL:
      this->on_special_(data.to<SpecialData>().get_special_command());
      break;
    default:
      return false;
  }

  this->publish_state();
  return true;
}

void MideaIR::on_control_(const ControlData &data) {
  if (!data.has_mode(climate::CLIMATE_MODE_FAN_ONLY))
    this->target_temperature = data.get_target_temperature();

  this->mode = data.get_mode();
  this->fan_mode = data.get_fan_mode();

  if (data.get_sleep_preset()) {
    this->preset = climate::CLIMATE_PRESET_SLEEP;
  } else if (this->preset == climate::CLIMATE_PRESET_SLEEP) {
    this->preset = climate::CLIMATE_PRESET_NONE;
  }
}

template<typename T, typename M> void toggle(T &val, const M &first, const M &second) {
  val = (val != second) ? second : first;
}

void MideaIR::on_special_(uint8_t command) {
  switch (command) {
    case VSWING_TOGGLE:
      toggle(this->swing_mode, climate::CLIMATE_SWING_OFF, climate::CLIMATE_SWING_VERTICAL);
      break;
    case TURBO_TOGGLE:
      toggle(this->preset, climate::CLIMATE_PRESET_NONE, climate::CLIMATE_PRESET_BOOST);
      break;
    default:
      ESP_LOGW(TAG, "Unhandled Midea IR special command: %d", command);
      return;
  }
}

}  // namespace midea_ir
}  // namespace esphome
