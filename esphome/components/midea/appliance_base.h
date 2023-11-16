#pragma once

#ifdef USE_ARDUINO

// MideaUART
#include <Appliance/ApplianceBase.h>
#include <Helpers/Logger.h>

// Include global defines
#include "esphome/core/defines.h"

#include "esphome/core/helpers.h"
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace midea {

class ApplianceBase : public Component, public Parented<dudanov::midea::ApplianceBase> {
 public:
  ApplianceBase(dudanov::midea::ApplianceBase *parent);

  /* UART communication */

  void set_uart_parent(uart::UARTComponent *parent) { this->uart_stream_.set_uart_parent(parent); }
  void set_period(uint32_t ms) { this->parent_->setPeriod(ms); }
  void set_response_timeout(uint32_t ms) { this->parent_->setTimeout(ms); }
  void set_request_attempts(uint32_t attempts) { this->parent_->setNumAttempts(attempts); }

  /* Component methods */

  void setup() override { this->parent_->setup(); }
  void loop() override { this->parent_->loop(); }
  float get_setup_priority() const override { return setup_priority::BEFORE_CONNECTION; }
  bool can_proceed() override {
    return this->parent_->getAutoconfStatus() != dudanov::midea::AutoconfStatus::AUTOCONF_PROGRESS;
  }

  void set_beeper_feedback(bool state) { this->parent_->setBeeper(state); }
  void set_autoconf(bool value) { this->parent_->setAutoconf(value); }

  virtual void on_status_change() = 0;

 protected:
  /* Stream from UART component */
  class UARTStream : public Stream {
   public:
    void set_uart_parent(uart::UARTComponent *uart) { this->uart_ = uart; }

    /* Stream interface implementation */

    int available() override { return this->uart_->available(); }
    int read() override {
      uint8_t data;
      this->uart_->read_byte(&data);
      return data;
    }
    int peek() override {
      uint8_t data;
      this->uart_->peek_byte(&data);
      return data;
    }
    size_t write(uint8_t data) override {
      this->uart_->write_byte(data);
      return 1;
    }
    size_t write(const uint8_t *data, size_t size) override {
      this->uart_->write_array(data, size);
      return size;
    }
    void flush() override { this->uart_->flush(); }

   protected:
    uart::UARTComponent *uart_;
  } uart_stream_;
};

}  // namespace midea
}  // namespace esphome

#endif  // USE_ARDUINO
