#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/modbus/modbus.h"

namespace esphome {
namespace hiking_meter {

#define HIKING_METER_SENSOR(name) \
 protected: \
  sensor::Sensor *name##_sensor_{nullptr}; \
\
 public: \
  void set_##name##_sensor(sensor::Sensor *(name)) { this->name##_sensor_ = name; }

class HikingMeter : public PollingComponent, public modbus::ModbusDevice {
 public:
  HIKING_METER_SENSOR(total_active_energy)
  HIKING_METER_SENSOR(export_active_energy)
  HIKING_METER_SENSOR(import_active_energy)
  HIKING_METER_SENSOR(voltage)
  HIKING_METER_SENSOR(current)
  HIKING_METER_SENSOR(active_power)
  HIKING_METER_SENSOR(reactive_power)
  HIKING_METER_SENSOR(power_factor)
  HIKING_METER_SENSOR(frequency)

  void update() override;

  void on_modbus_data(const std::vector<uint8_t> &data) override;

  void dump_config() override;
};

}  // namespace hiking_meter
}  // namespace esphome
