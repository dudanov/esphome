#include "midea_dongle.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace midea_dongle {

static const char *const TAG = "midea_dongle";

void MideaDongle::loop() {
  while (this->reader_.read(this)) {
    ESP_LOGD(TAG, "RX: %s", this->reader_.to_string().c_str());
    if (!this->reader_.is_valid()) {
      ESP_LOGW(TAG, "RX: frame check failed!");
      continue;
    }
    // need send response to QUERY_NETWORK request
    if (this->reader_.get_type() == QUERY_NETWORK) {
      this->notify_.set_type(QUERY_NETWORK);
      this->need_notify_ = true;
      break;
    }
    // ignore answer for NETWORK_NOTIFY request
    if (this->reader_.get_type() == NETWORK_NOTIFY)
      continue;
    // other frames send to appliance
    if (this->appliance_ != nullptr)
      this->appliance_->on_frame(this->reader_);
  }
}

void MideaDongle::update() {
  const bool is_conn = WiFi.isConnected();
  uint8_t wifi_strength = 0;
  if (!this->rssi_timer_) {
    if (is_conn)
      wifi_strength = 4;
  } else if (is_conn) {
    if (--this->rssi_timer_) {
      wifi_strength = this->notify_.get_signal_strength();
    } else {
      this->rssi_timer_ = 60;
      const int32_t dbm = WiFi.RSSI();
      if (dbm > -63)
        wifi_strength = 4;
      else if (dbm > -75)
        wifi_strength = 3;
      else if (dbm > -88)
        wifi_strength = 2;
      else if (dbm > -100)
        wifi_strength = 1;
    }
  } else {
    this->rssi_timer_ = 1;
  }
  if (this->notify_.is_connected() != is_conn) {
    this->notify_.set_connected(is_conn);
    this->need_notify_ = true;
  }
  if (this->notify_.get_signal_strength() != wifi_strength) {
    this->notify_.set_signal_strength(wifi_strength);
    this->need_notify_ = true;
  }
  if (!--this->notify_timer_) {
    this->notify_.set_type(NETWORK_NOTIFY);
    this->need_notify_ = true;
  }
  if (this->need_notify_) {
    ESP_LOGD(TAG, "TX: notify WiFi STA %s, signal strength %d", is_conn ? "connected" : "not connected", wifi_strength);
    this->need_notify_ = false;
    this->notify_timer_ = 600;
    this->notify_.finalize();
    this->write_frame(this->notify_);
    return;
  }
  if (this->appliance_ != nullptr)
    this->appliance_->on_update();
}

void MideaDongle::write_frame(const Frame &frame) {
  this->write_array(frame.data(), frame.size());
  ESP_LOGD(TAG, "TX: %s", frame.to_string().c_str());
}

void MideaDongle::transmit_ir(remote_base::MideaData &data) {
  if (this->transmitter_ == nullptr) {
    ESP_LOGW(TAG, "To transmit IR you need remote_transmitter component in your YAML configuration.");
    return;
  }
  data.finalize();
  ESP_LOGD(TAG, "Sending Midea IR data: %s", data.to_string().c_str());
  auto transmit = this->transmitter_->transmit();
  remote_base::MideaProtocol().encode(transmit.get_data(), data);
  transmit.perform();
}

}  // namespace midea_dongle
}  // namespace esphome
