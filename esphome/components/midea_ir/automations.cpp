#include "automations.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace midea_ir {

void FollowMeData::set_temperature(float temperature, const bool use_fahrenheit) {
  long min = C_MIN, max = C_MAX;

  if (use_fahrenheit) {
    min = F_MIN, max = F_MAX;
    temperature = celsius_to_fahrenheit(temperature);
  }

  const auto temp = clamp(lroundf(temperature), min, max);
  this->set_value_(4, temp - min + 1);
  this->set_use_fahrenheit(use_fahrenheit);
}

float FollowMeData::get_temperature() const {
  const uint8_t temp = this->get_value_(4);

  if (this->get_use_fahrenheit())
    return round_half(fahrenheit_to_celsius(temp + F_MIN - 1));

  return temp + C_MIN - 1;
}

}  // namespace midea_ir
}  // namespace esphome
