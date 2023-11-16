#pragma once

#include "esphome/components/remote_base/midea_protocol.h"

namespace esphome {
namespace midea_ir {

using remote_base::MideaData;

constexpr float round_half(float input) { return std::round(input * 2.0f) * 0.5f; }

/* DATA TYPES */

enum SpecialCommand : uint8_t {
  /* Used in Climate */

  VSWING_TOGGLE = 0x02,
  TURBO_TOGGLE = 0x09,

  /* Used in Buttons */

  VSWING_STEP = 0x01,
  DISPLAY_TOGGLE = 0x08,

  /* Extra unused */

  ECO = 0x02,
  SELF_CLEAN = 0x0D,
  FREEZE_PROTECTION = 0x0F,
  QUIET_ON = 0x12,
  QUIET_OFF = 0x13,
};

class FollowMeData : public MideaData {
 public:
  // Default constructor (temp: 30C, beeper: off)
  FollowMeData() : MideaData({MIDEA_TYPE_FOLLOW_ME, 0x82, 0x48, 0x7F, 0x1F}) {}
  FollowMeData(const MideaData &data) : MideaData(data) {}

  void set_beeper(bool beeper) { this->set_mask_(3, beeper, 128); }
  void set_temperature(float temperature, bool use_fahrenheit);
  float get_temperature() const;
  void set_use_fahrenheit(bool use_fahrenheit) { this->set_mask_(2, use_fahrenheit, 32); }
  bool get_use_fahrenheit() const { return this->get_value_(2, 32); }

 protected:
  static const uint8_t C_MIN = 0;
  static const uint8_t C_MAX = 37;
  static const uint8_t F_MIN = 32;
  static const uint8_t F_MAX = 99;
};

class SpecialData : public MideaData {
 public:
  SpecialData(uint8_t command) : MideaData({MIDEA_TYPE_SPECIAL, command, 0xFF, 0xFF, 0xFF}) {}
  SpecialData(const MideaData &data) : MideaData(data) {}
  uint8_t get_special_command() const { return this->get_value_(1); }
  static void transmit(remote_base::RemoteTransmitterBase *transmitter, uint8_t command) {
    SpecialData(command).transmit_cs(transmitter);
  }
};

class VerticalSwingStepData : public SpecialData {
 public:
  VerticalSwingStepData() : SpecialData(VSWING_STEP) {}
};

class VerticalSwingToggleData : public SpecialData {
 public:
  VerticalSwingToggleData() : SpecialData(VSWING_TOGGLE) {}
};

class DisplayToggleData : public SpecialData {
 public:
  DisplayToggleData() : SpecialData(DISPLAY_TOGGLE) {}
};

class TurboToggleData : public SpecialData {
 public:
  TurboToggleData() : SpecialData(TURBO_TOGGLE) {}
};

/* ACTIONS */

template<typename... Ts> class MideaRemoteAction : public remote_base::RemoteTransmittable, public Action<Ts...> {
 public:
  void play(Ts... x) override { this->get_midea_data(x...).transmit_cs(this->transmitter_); }

 protected:
  virtual MideaData get_midea_data(Ts... x) = 0;
};

template<typename... Ts> class DisplayToggleAction : public MideaRemoteAction<Ts...> {
 protected:
  MideaData get_midea_data(Ts... x) override { return DisplayToggleData(); }
};

template<typename... Ts> class FollowMeAction : public MideaRemoteAction<Ts...> {
  TEMPLATABLE_VALUE(float, temperature)
  TEMPLATABLE_VALUE(bool, use_fahrenheit)
  TEMPLATABLE_VALUE(bool, beeper)

 protected:
  MideaData get_midea_data(Ts... x) override {
    FollowMeData data;
    data.set_beeper(this->beeper_.value(x...));
    data.set_temperature(this->temperature_.value(x...), this->use_fahrenheit_.value(x...));
    return data;
  }
};

template<typename... Ts> class SwingStepAction : public MideaRemoteAction<Ts...> {
 protected:
  MideaData get_midea_data(Ts... x) override { return VerticalSwingStepData(); }
};

}  // namespace midea_ir
}  // namespace esphome
