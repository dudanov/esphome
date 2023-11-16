#pragma once

#include <type_traits>
#include "esphome/core/optional.h"

namespace esphome {
namespace climate_ir {

template<typename T, typename M = T> class ControlHelper {
 public:
  template<typename T1 = T, typename T2 = M, std::enable_if_t<std::is_same_v<T1, T2>, bool> = true>
  ControlHelper(M &property, const optional<T> &change) : property_(property), value_(change.value_or(property)) {}

  template<typename T1 = T, typename T2 = M, std::enable_if_t<std::is_same_v<optional<T1>, T2>, bool> = true>
  ControlHelper(M &property, const optional<T> &update, const T &default_ = {}) : property_(property) {
    if (!this->property_.has_value())
      this->set_property(default_);
    this->value_ = update.value_or(this->get_property());
  }

  template<typename T1 = T, typename T2 = M, std::enable_if_t<std::is_same_v<T1, T2>, bool> = true>
  const T &get_property() const {
    return this->property_;
  }

  template<typename T1 = T, typename T2 = M, std::enable_if_t<std::is_same_v<optional<T1>, T2>, bool> = true>
  const T &get_property() const {
    return *this->property_;
  }

  const T &get_value() const { return this->value_; }

  void set_property(const T &value) { this->property_ = value; }
  void set_value(const T &value) { this->value_ = value; }

  bool is_committed() const { return this->get_property() == this->get_value(); }
  bool has_property(const T &value) const { return this->get_property() == value; }
  bool has_value(const T &value) const { return this->get_value() == value; }
  bool has_uncommitted_value(const T &value) const { return this->has_value(value) && !this->has_property(value); }
  bool is_from_to(const T &from, const T &to) const { return this->has_property(from) && this->has_value(to); }
  bool is_switch(const T &first, const T &second) const {
    return this->is_from_to(first, second) || this->is_from_to(second, first);
  }

  void commit() { this->set_property(this->get_value()); }
  void commit(const T &value) {
    this->set_value(value);
    this->set_property(value);
  }

 protected:
  M &property_;
  T value_;
};

}  // namespace climate_ir
}  // namespace esphome
