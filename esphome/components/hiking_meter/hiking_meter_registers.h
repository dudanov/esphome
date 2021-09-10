#pragma once

namespace esphome {
namespace hiking_meter {

static const float THREE_DEC_UNIT = 0.001f;
static const float TWO_DEC_UNIT = 0.01f;
static const float ONE_DEC_UNIT = 0.1f;
static const float NO_DEC_UNIT = 1.0f;

/* PHASE STATUS REGISTERS */
static const uint16_t HIKING_TOTAL_ACTIVE_ENERGY = 0x0000;
static const uint16_t HIKING_EXPORT_ACTIVE_ENERGY = 0x0008;
static const uint16_t HIKING_IMPORT_ACTIVE_ENERGY = 0x000A;
static const uint16_t HIKING_VOLTAGE = 0x000C;
static const uint16_t HIKING_CURRENT = 0x000D;
static const uint16_t HIKING_ACTIVE_POWER = 0x000E;
static const uint16_t HIKING_REACTIVE_POWER = 0x000F;
static const uint16_t HIKING_POWER_FACTOR = 0x0010;
static const uint16_t HIKING_FREQUENCY = 0x0011;

}  // namespace hiking_meter
}  // namespace esphome
