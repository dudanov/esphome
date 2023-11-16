#pragma once

#include "esphome/components/remote_base/remote_base.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace midea_ir {

class MideaRemoteSensors : public remote_base::RemoteReceiverListener {
 public:
  bool on_receive(remote_base::RemoteReceiveData src) override;

  void set_fm_sensor(sensor::Sensor *sensor) { this->fm_sensor_ = sensor; }

 protected:
  sensor::Sensor *fm_sensor_{nullptr};
};

}  // namespace midea_ir
}  // namespace esphome
