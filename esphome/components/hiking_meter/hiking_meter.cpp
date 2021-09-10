#include "hiking_meter.h"
#include "hiking_meter_registers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace hiking_meter {

static const char *const TAG = "hiking_meter";

static const uint8_t MODBUS_CMD_READ_IN_REGISTERS = 0x03;
static const uint8_t MODBUS_REGISTER_COUNT = 18;  // 18 x 16-bit registers

void HikingMeter::on_modbus_data(const std::vector<uint8_t> &data) {
  if (data.size() < MODBUS_REGISTER_COUNT * 2) {
    ESP_LOGW(TAG, "Invalid size for HikingMeter!");
    return;
  }

  auto hiking_meter_get_float16 = [&](size_t i, float unit) -> float {
    return static_cast<float>(encode_uint16(data[i], data[i + 1])) * unit;
  };

  auto hiking_meter_get_float32 = [&](size_t i, float unit) -> float {
    return static_cast<float>(encode_uint32(data[i + 2], data[i + 3], data[i], data[i + 1])) * unit;
  };

  float total_active_energy = hiking_meter_get_float32(HIKING_TOTAL_ACTIVE_ENERGY * 2, TWO_DEC_UNIT);
  float export_active_energy = hiking_meter_get_float32(HIKING_EXPORT_ACTIVE_ENERGY * 2, TWO_DEC_UNIT);
  float import_active_energy = hiking_meter_get_float32(HIKING_IMPORT_ACTIVE_ENERGY * 2, TWO_DEC_UNIT);
  float voltage = hiking_meter_get_float16(HIKING_VOLTAGE * 2, ONE_DEC_UNIT);
  float current = hiking_meter_get_float16(HIKING_CURRENT * 2, TWO_DEC_UNIT);
  float active_power = hiking_meter_get_float16(HIKING_ACTIVE_POWER * 2, NO_DEC_UNIT);
  float reactive_power = hiking_meter_get_float16(HIKING_REACTIVE_POWER * 2, NO_DEC_UNIT);
  float power_factor = hiking_meter_get_float16(HIKING_POWER_FACTOR * 2, THREE_DEC_UNIT);
  float frequency = hiking_meter_get_float16(HIKING_FREQUENCY * 2, TWO_DEC_UNIT);

  if (this->total_active_energy_sensor_ != nullptr)
    this->total_active_energy_sensor_->publish_state(total_active_energy);
  if (this->export_active_energy_sensor_ != nullptr)
    this->export_active_energy_sensor_->publish_state(export_active_energy);
  if (this->import_active_energy_sensor_ != nullptr)
    this->import_active_energy_sensor_->publish_state(import_active_energy);
  if (this->voltage_sensor_ != nullptr)
    this->voltage_sensor_->publish_state(voltage);
  if (this->current_sensor_ != nullptr)
    this->current_sensor_->publish_state(current);
  if (this->active_power_sensor_ != nullptr)
    this->active_power_sensor_->publish_state(active_power);
  if (this->reactive_power_sensor_ != nullptr)
    this->reactive_power_sensor_->publish_state(reactive_power);
  if (this->power_factor_sensor_ != nullptr)
    this->power_factor_sensor_->publish_state(power_factor);
  if (this->frequency_sensor_ != nullptr)
    this->frequency_sensor_->publish_state(frequency);
}

void HikingMeter::update() { this->send(MODBUS_CMD_READ_IN_REGISTERS, 0, MODBUS_REGISTER_COUNT); }
void HikingMeter::dump_config() {
  ESP_LOGCONFIG(TAG, "HIKING Meter:");
  ESP_LOGCONFIG(TAG, "  Address: 0x%02X", this->address_);
  LOG_SENSOR("  ", "Total Active Energy", this->total_active_energy_sensor_);
  LOG_SENSOR("  ", "Export Active Energy", this->export_active_energy_sensor_);
  LOG_SENSOR("  ", "Import Active Energy", this->import_active_energy_sensor_);
  LOG_SENSOR("  ", "Active Power", this->active_power_sensor_);
  LOG_SENSOR("  ", "Reactive Power", this->reactive_power_sensor_);
  LOG_SENSOR("  ", "Voltage", this->voltage_sensor_);
  LOG_SENSOR("  ", "Current", this->current_sensor_);
  LOG_SENSOR("  ", "Power Factor", this->power_factor_sensor_);
  LOG_SENSOR("  ", "Frequency", this->frequency_sensor_);
}

}  // namespace hiking_meter
}  // namespace esphome
