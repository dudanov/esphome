#pragma once

#ifdef USE_ARDUINO

// MideaUART
#include <Appliance/AirConditioner/AirConditioner.h>

#include "air_conditioner.h"

namespace esphome {
namespace midea {
namespace ac {

using MideaMode = dudanov::midea::ac::Mode;
using MideaSwingMode = dudanov::midea::ac::SwingMode;
using MideaFanMode = dudanov::midea::ac::FanMode;
using MideaPreset = dudanov::midea::ac::Preset;

class Constants {
 public:
  static const char *const TAG;
  static const std::string FREEZE_PROTECTION;
  static const std::string SILENT;
  static const std::string TURBO;
};

MideaMode to_midea_mode(ClimateMode mode);
ClimateMode to_climate_mode(MideaMode mode);
MideaSwingMode to_midea_swing_mode(ClimateSwingMode mode);
ClimateSwingMode to_climate_swing_mode(MideaSwingMode mode);
MideaPreset to_midea_preset(ClimatePreset preset);
MideaPreset to_midea_preset(const std::string &preset);
bool is_custom_midea_preset(MideaPreset preset);
ClimatePreset to_climate_preset(MideaPreset preset);
const std::string &to_custom_climate_preset(MideaPreset preset);
MideaFanMode to_midea_fan_mode(ClimateFanMode fan_mode);
MideaFanMode to_midea_fan_mode(const std::string &fan_mode);
bool is_custom_midea_fan_mode(MideaFanMode fan_mode);
ClimateFanMode to_climate_fan_mode(MideaFanMode fan_mode);
const std::string &to_custom_climate_fan_mode(MideaFanMode fan_mode);
void to_climate_traits(ClimateTraits &traits, const dudanov::midea::ac::Capabilities &capabilities);

}  // namespace ac
}  // namespace midea
}  // namespace esphome

#endif  // USE_ARDUINO
