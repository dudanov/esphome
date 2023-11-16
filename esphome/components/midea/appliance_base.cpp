#ifdef USE_ARDUINO

#include "appliance_base.h"
#include "esphome/core/log.h"

namespace esphome {
namespace midea {

ApplianceBase::ApplianceBase(dudanov::midea::ApplianceBase *parent) : Parented(parent) {
  parent->setStream(&this->uart_stream_);
  parent->addOnStateCallback(std::bind(&ApplianceBase::on_status_change, this));
  dudanov::midea::ApplianceBase::setLogger(
      [](int level, const char *tag, int line, const String &format, va_list args) {
        esp_log_vprintf_(level, tag, line, format.c_str(), args);
      });
}

}  // namespace midea
}  // namespace esphome

#endif  // USE_ARDUINO
