#pragma once

#ifdef USE_ARDUINO

#include "air_conditioner.h"
#include "esphome/core/automation.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace midea {
namespace ac {

template<typename... Ts> class MideaActionBase : public Action<Ts...>, public Parented<AirConditioner> {};

template<typename... Ts> class DisplayToggleAction : public MideaActionBase<Ts...> {
 public:
  void play(Ts... x) override { this->parent_->do_display_toggle(); }
};

template<typename... Ts> class BeeperOnAction : public MideaActionBase<Ts...> {
 public:
  void play(Ts... x) override { this->parent_->set_beeper_feedback(true); }
};

template<typename... Ts> class BeeperOffAction : public MideaActionBase<Ts...> {
 public:
  void play(Ts... x) override { this->parent_->set_beeper_feedback(false); }
};

template<typename... Ts> class PowerOnAction : public MideaActionBase<Ts...> {
 public:
  void play(Ts... x) override { this->parent_->do_power_on(); }
};

template<typename... Ts> class PowerOffAction : public MideaActionBase<Ts...> {
 public:
  void play(Ts... x) override { this->parent_->do_power_off(); }
};

template<typename... Ts> class PowerToggleAction : public MideaActionBase<Ts...> {
 public:
  void play(Ts... x) override { this->parent_->do_power_toggle(); }
};

class BeeperSwitch : public switch_::Switch, public Parented<AirConditioner> {
  void write_state(bool state) override { this->parent_->set_beeper_feedback(state); }
};

}  // namespace ac
}  // namespace midea
}  // namespace esphome

#endif  // USE_ARDUINO
