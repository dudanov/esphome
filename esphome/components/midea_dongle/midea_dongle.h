#pragma once
#include "esphome/core/component.h"
#include "esphome/components/wifi/wifi_component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/remote_base/midea_protocol.h"
#include "esphome/components/remote_transmitter/remote_transmitter.h"
#include "midea_frame.h"

namespace esphome {
namespace midea_dongle {

enum MideaApplianceType : uint8_t { DEHUMIDIFIER = 0xA1, AIR_CONDITIONER = 0xAC, BROADCAST = 0xFF };
enum MideaMessageType : uint8_t {
  DEVICE_CONTROL = 0x02,
  DEVICE_QUERY = 0x03,
  NETWORK_NOTIFY = 0x0D,
  QUERY_NETWORK = 0x63,
};

struct MideaAppliance {
  /// Calling on update event
  virtual void on_update() = 0;
  /// Calling on frame receive event
  virtual void on_frame(const Frame &frame) = 0;
};

class MideaDongle : public PollingComponent, public uart::UARTDevice {
 public:
  MideaDongle() : PollingComponent(1000) {}
  float get_setup_priority() const override { return setup_priority::LATE; }
  void update() override;
  void loop() override;
  void set_appliance(MideaAppliance *app) { this->appliance_ = app; }
  void set_transmitter(remote_transmitter::RemoteTransmitterComponent *transmitter) {
    this->transmitter_ = transmitter;
  }
  void transmit_ir(remote_base::MideaData &data);
  void use_strength_icon(bool state) { this->rssi_timer_ = state; }
  void write_frame(const Frame &frame);

 protected:
  MideaAppliance *appliance_{nullptr};
  remote_transmitter::RemoteTransmitterComponent *transmitter_{nullptr};
  NotifyFrame notify_;
  FrameReader reader_;
  unsigned notify_timer_{1};
  uint8_t rssi_timer_{0};
  bool need_notify_{false};
};

}  // namespace midea_dongle
}  // namespace esphome
