#include "coolix.h"
#include "esphome/components/remote_base/coolix_protocol.h"
#include "esphome/core/log.h"

namespace esphome {
namespace coolix {

static const char *const TAG = "coolix.climate";

static const uint32_t COOLIX_OFF = 0xB27BE0;
static const uint32_t COOLIX_SWING = 0xB26BE0;
static const uint32_t COOLIX_VSWING_STEP = 0xB20FE0;
static const uint32_t COOLIX_HSWING_STEP = 0xB5F5A2;
static const uint32_t COOLIX_SLEEP = 0xB2E003;
static const uint32_t COOLIX_TURBO = 0xB5F5A2;
static const uint32_t COOLIX_LED = 0xB5F5A5;

using remote_base::CoolixData;

class ControlData : protected CoolixData {
  static CoolixData make_poweroff() { return COOLIX_OFF; }
  static CoolixData make_swing() { return COOLIX_SWING; }
};

void transmit_coolix(remote_base::RemoteTransmitterBase *transmitter, const CoolixData &data) {
  auto transmit = transmitter->transmit();
  remote_base::CoolixProtocol().encode(transmit.get_data(), data);
  transmit.perform();
}

/*
  RECEIVER NOTES:
  1. `main` control command should only be looked for in `CoolixData.second`;
  2. `TURBO` command always `stricted`;
  3. `SLEEP` command always in `CoolixData.first` and used with others commands (`unstricted`);
  4. `SLEEP` and `TURBO` presets are mutually exclusive. Except when `SLEEP` used with `SWING`,
     in which `SLEEP` does not disable `TURBO`;
  5. `SWING` command should only be looked for in `CoolixData.second`;
  6. `SWING_STEP` command always in `CoolixData.first` and consist only from one command;
*/

// On, 25C, Mode: Auto, Fan: Auto, Zone Follow: Off, Sensor Temp: Ignore.

// Mode
static const uint32_t COOLIX_MODE_MASK = 0b1100;
static const uint8_t COOLIX_COOL = 0b0000;
static const uint8_t COOLIX_DRY_FAN = 0b0100;
static const uint8_t COOLIX_AUTO = 0b1000;
static const uint8_t COOLIX_HEAT = 0b1100;

// Fan
static const uint32_t COOLIX_FAN_MASK = 0xF000;
static const uint32_t COOLIX_FAN_MODE_AUTO_DRY = 0x1000;
static const uint32_t COOLIX_FAN_AUTO = 0xB000;
static const uint32_t COOLIX_FAN_MIN = 0x9000;
static const uint32_t COOLIX_FAN_MED = 0x5000;
static const uint32_t COOLIX_FAN_MAX = 0x3000;

// Temperature
static const uint32_t COOLIX_TEMP_MASK = 0b11110000;
static const uint8_t COOLIX_FAN_TEMP_CODE = 0b11100000;  // Part of Fan Mode.
static const uint8_t COOLIX_TEMP_MAP[] = {
    0x00, 0x10, 0x30, 0x20, 0x60, 0x70, 0x50, 0x40, 0xC0, 0xD0, 0x90, 0x80, 0xA0, 0xB0,
};

void CoolixClimate::control(const climate::ClimateCall &call) {
  send_swing_cmd_ = call.get_swing_mode().has_value();
  // swing resets after unit powered off
  if (call.get_mode().has_value() && *call.get_mode() == climate::CLIMATE_MODE_OFF)
    this->swing_mode = climate::CLIMATE_SWING_OFF;
  climate_ir::ClimateIR::control(call);
}

void CoolixClimate::transmit_state() {
  uint32_t remote_state = 0xB20F00;

  if (send_swing_cmd_) {
    send_swing_cmd_ = false;
    remote_state = COOLIX_SWING;
  } else {
    switch (this->mode) {
      case climate::CLIMATE_MODE_COOL:
        remote_state |= COOLIX_COOL;
        break;
      case climate::CLIMATE_MODE_HEAT:
        remote_state |= COOLIX_HEAT;
        break;
      case climate::CLIMATE_MODE_HEAT_COOL:
        remote_state |= COOLIX_AUTO;
        break;
      case climate::CLIMATE_MODE_FAN_ONLY:
      case climate::CLIMATE_MODE_DRY:
        remote_state |= COOLIX_DRY_FAN;
        break;
      case climate::CLIMATE_MODE_OFF:
      default:
        remote_state = COOLIX_OFF;
        break;
    }
    if (this->mode != climate::CLIMATE_MODE_OFF) {
      if (this->mode != climate::CLIMATE_MODE_FAN_ONLY) {
        uint8_t temp = lroundf(clamp<float>(this->target_temperature, COOLIX_TEMP_MIN, COOLIX_TEMP_MAX));
        remote_state |= COOLIX_TEMP_MAP[temp - COOLIX_TEMP_MIN];
      } else {
        remote_state |= COOLIX_FAN_TEMP_CODE;
      }
      if (this->mode == climate::CLIMATE_MODE_HEAT_COOL || this->mode == climate::CLIMATE_MODE_DRY) {
        this->fan_mode = climate::CLIMATE_FAN_AUTO;
        remote_state |= COOLIX_FAN_MODE_AUTO_DRY;
      } else {
        switch (this->fan_mode.value()) {
          case climate::CLIMATE_FAN_HIGH:
            remote_state |= COOLIX_FAN_MAX;
            break;
          case climate::CLIMATE_FAN_MEDIUM:
            remote_state |= COOLIX_FAN_MED;
            break;
          case climate::CLIMATE_FAN_LOW:
            remote_state |= COOLIX_FAN_MIN;
            break;
          case climate::CLIMATE_FAN_AUTO:
          default:
            remote_state |= COOLIX_FAN_AUTO;
            break;
        }
      }
    }
  }
  ESP_LOGV(TAG, "Sending coolix code: 0x%06" PRIX32, remote_state);

  this->transmit_<remote_base::CoolixProtocol>(remote_state);
  //auto transmit = this->transmitter_->transmit();
  //auto *data = transmit.get_data();
  //remote_base::CoolixProtocol().encode(data, remote_state);
  //transmit.perform();
}

template<typename T, typename M> void toggle(T &val, const M &first, const M &second) {
  val = (val == first) ? second : first;
}

template<typename T, typename M> void update_property(T &property, const M &value, bool &is_updated) {
  if (property == value)
    return;
  property = value;
  is_updated = true;
}

bool CoolixClimate::on_coolix(climate::Climate *parent, remote_base::RemoteReceiveData data) {
  auto decoded = remote_base::CoolixProtocol().decode(data);
  if (!decoded.has_value())
    return false;
  // Decoded remote state y 3 bytes long code.
  uint32_t remote_state = (*decoded).second;
  ESP_LOGV(TAG, "Decoded 0x%06" PRIX32, remote_state);

  const uint8_t hdr = remote_state >> 16;

  if (hdr != 0xB2 && hdr != 0xB5)
    return false;

  bool is_updated = false;

  if (remote_state == COOLIX_OFF) {
    /* OFF MODE. ANY PRESETS CLEAR. */
    update_property(parent->mode, climate::CLIMATE_MODE_OFF, is_updated);
    update_property(parent->preset, climate::CLIMATE_PRESET_NONE, is_updated);

  } else if (remote_state == COOLIX_SWING) {
    /* TOGGLE VERTICAL SWING MODE. PRESERVE IT STATE EVEN IN OFF MODE. */
    toggle(parent->swing_mode, climate::CLIMATE_SWING_OFF, climate::CLIMATE_SWING_VERTICAL);
    is_updated = true;

  } else if (remote_state == COOLIX_SLEEP) {
    /* SLEEP PRESET. NOT WORK IN DRY OR FAN_ONLY MODES. */
    if (parent->mode != climate::CLIMATE_MODE_DRY && parent->mode != climate::CLIMATE_MODE_FAN_ONLY) {
      update_property(parent->preset, climate::CLIMATE_PRESET_SLEEP, is_updated);
      update_property(parent->fan_mode, climate::CLIMATE_FAN_AUTO, is_updated);
    }

  } else if (remote_state == COOLIX_TURBO) {
    /* TOGGLE BOOST PRESET. WORK ONLY IN COOL AND HEAT MODES. */
    if (parent->mode == climate::CLIMATE_MODE_COOL || parent->mode == climate::CLIMATE_MODE_HEAT) {
      toggle(parent->preset, climate::CLIMATE_PRESET_BOOST, climate::CLIMATE_PRESET_NONE);
      parent->fan_mode = climate::CLIMATE_FAN_AUTO;
      is_updated = true;
    }

  } else {
    if ((remote_state & COOLIX_MODE_MASK) == COOLIX_HEAT) {
      parent->mode = climate::CLIMATE_MODE_HEAT;
    } else if ((remote_state & COOLIX_MODE_MASK) == COOLIX_AUTO) {
      parent->mode = climate::CLIMATE_MODE_HEAT_COOL;
    } else if ((remote_state & COOLIX_MODE_MASK) == COOLIX_DRY_FAN) {
      if ((remote_state & COOLIX_FAN_MASK) == COOLIX_FAN_MODE_AUTO_DRY) {
        parent->mode = climate::CLIMATE_MODE_DRY;
      } else {
        parent->mode = climate::CLIMATE_MODE_FAN_ONLY;
      }
    } else
      parent->mode = climate::CLIMATE_MODE_COOL;

    // Fan Speed
    if ((remote_state & COOLIX_FAN_AUTO) == COOLIX_FAN_AUTO || parent->mode == climate::CLIMATE_MODE_HEAT_COOL ||
        parent->mode == climate::CLIMATE_MODE_DRY) {
      parent->fan_mode = climate::CLIMATE_FAN_AUTO;
    } else if ((remote_state & COOLIX_FAN_MIN) == COOLIX_FAN_MIN) {
      parent->fan_mode = climate::CLIMATE_FAN_LOW;
    } else if ((remote_state & COOLIX_FAN_MED) == COOLIX_FAN_MED) {
      parent->fan_mode = climate::CLIMATE_FAN_MEDIUM;
    } else if ((remote_state & COOLIX_FAN_MAX) == COOLIX_FAN_MAX) {
      parent->fan_mode = climate::CLIMATE_FAN_HIGH;
    }

    // Temperature
    uint8_t temperature_code = remote_state & COOLIX_TEMP_MASK;
    for (unsigned idx = 0; idx != sizeof(COOLIX_TEMP_MAP); ++idx) {
      if (COOLIX_TEMP_MAP[idx] == temperature_code)
        parent->target_temperature = idx + COOLIX_TEMP_MIN;
    }
  }
  parent->publish_state();

  return true;
}

}  // namespace coolix
}  // namespace esphome
