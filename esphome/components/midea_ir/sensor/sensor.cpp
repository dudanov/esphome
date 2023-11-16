#include "sensor.h"
#include "esphome/components/midea_ir/automations.h"

namespace esphome {
namespace midea_ir {

bool MideaRemoteSensors::on_receive(remote_base::RemoteReceiveData src) {
  auto opt = remote_base::MideaProtocol().decode(src);
  if (!opt.has_value())
    return false;
  const auto &data = opt.value();
  switch (data.type()) {
    case remote_base::MideaData::MIDEA_TYPE_FOLLOW_ME:
      if (this->fm_sensor_ != nullptr)
        this->fm_sensor_->publish_state(data.to<FollowMeData>().get_temperature());
      return true;
    default:
      return false;
  }
}

}  // namespace midea_ir
}  // namespace esphome
