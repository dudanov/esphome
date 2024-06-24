from esphome.core import coroutine
from esphome import automation
from esphome.components import climate, sensor, uart
from esphome.components.remote_base import CONF_TRANSMITTER_ID
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import (
    CONF_AUTOCONF,
    CONF_BEEPER,
    CONF_CUSTOM_FAN_MODES,
    CONF_CUSTOM_PRESETS,
    CONF_ID,
    CONF_NUM_ATTEMPTS,
    CONF_PERIOD,
    CONF_SUPPORTED_MODES,
    CONF_SUPPORTED_PRESETS,
    CONF_SUPPORTED_SWING_MODES,
    CONF_TIMEOUT,
    CONF_USE_FAHRENHEIT,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_HUMIDITY,
    ENTITY_CATEGORY_CONFIG,
    ICON_POWER,
    ICON_THERMOMETER,
    ICON_WATER_PERCENT,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_PERCENT,
    UNIT_WATT,
)
from esphome.components.climate import (
    ClimateMode,
    ClimatePreset,
    ClimateSwingMode,
)
from . import midea_ac_ns, register_midea, midea_parented_schema
from esphome.components.switch import (
    Switch,
    new_switch,
    switch_schema,
)

AUTO_LOAD = ["climate", "sensor", "switch"]
DEPENDENCIES = ["uart", "wifi"]

CONF_HUMIDITY_SETPOINT = "humidity_setpoint"
CONF_OUTDOOR_TEMPERATURE = "outdoor_temperature"
CONF_POWER_USAGE = "power_usage"


AirConditioner = midea_ac_ns.class_("AirConditioner", climate.Climate, cg.Component)
BeeperSwitch = midea_ac_ns.class_("BeeperSwitch", Switch)
Capabilities = midea_ac_ns.namespace("Constants")

BEEPER_SWITCH_SCHEMA = switch_schema(
    BeeperSwitch,
    entity_category=ENTITY_CATEGORY_CONFIG,
    icon="mdi:volume-source",
    default_restore_mode="RESTORE_DEFAULT_ON",
).extend(midea_parented_schema(AirConditioner))


async def new_midea_switch(config):
    var = await new_switch(config)
    await register_midea(var, config)


ALLOWED_CLIMATE_MODES = {
    "HEAT_COOL": ClimateMode.CLIMATE_MODE_HEAT_COOL,
    "COOL": ClimateMode.CLIMATE_MODE_COOL,
    "HEAT": ClimateMode.CLIMATE_MODE_HEAT,
    "DRY": ClimateMode.CLIMATE_MODE_DRY,
    "FAN_ONLY": ClimateMode.CLIMATE_MODE_FAN_ONLY,
}

ALLOWED_CLIMATE_PRESETS = {
    "ECO": ClimatePreset.CLIMATE_PRESET_ECO,
    "BOOST": ClimatePreset.CLIMATE_PRESET_BOOST,
    "SLEEP": ClimatePreset.CLIMATE_PRESET_SLEEP,
}

ALLOWED_CLIMATE_SWING_MODES = {
    "BOTH": ClimateSwingMode.CLIMATE_SWING_BOTH,
    "VERTICAL": ClimateSwingMode.CLIMATE_SWING_VERTICAL,
    "HORIZONTAL": ClimateSwingMode.CLIMATE_SWING_HORIZONTAL,
}

CUSTOM_FAN_MODES = {
    "SILENT": Capabilities.SILENT,
    "TURBO": Capabilities.TURBO,
}

CUSTOM_PRESETS = {
    "FREEZE_PROTECTION": Capabilities.FREEZE_PROTECTION,
}

validate_modes = cv.enum(ALLOWED_CLIMATE_MODES, upper=True)
validate_presets = cv.enum(ALLOWED_CLIMATE_PRESETS, upper=True)
validate_swing_modes = cv.enum(ALLOWED_CLIMATE_SWING_MODES, upper=True)
validate_custom_fan_modes = cv.enum(CUSTOM_FAN_MODES, upper=True)
validate_custom_presets = cv.enum(CUSTOM_PRESETS, upper=True)

CONFIG_SCHEMA = cv.All(
    climate.CLIMATE_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(AirConditioner),
            cv.Optional(CONF_PERIOD, default="1s"): cv.time_period,
            cv.Optional(CONF_TIMEOUT, default="2s"): cv.time_period,
            cv.Optional(CONF_NUM_ATTEMPTS, default=3): cv.int_range(min=1, max=5),
            cv.OnlyWith(CONF_TRANSMITTER_ID, "remote_transmitter", True): cv.boolean,
            cv.Optional(CONF_AUTOCONF, default=True): cv.boolean,
            cv.Optional(CONF_BEEPER): BEEPER_SWITCH_SCHEMA,
            # TODO: auto-import to 'midea_ir' automations.
            cv.Optional(CONF_USE_FAHRENHEIT, default=False): cv.boolean,
            cv.Optional(CONF_SUPPORTED_MODES): cv.ensure_list(validate_modes),
            cv.Optional(CONF_SUPPORTED_SWING_MODES): cv.ensure_list(
                validate_swing_modes
            ),
            cv.Optional(CONF_SUPPORTED_PRESETS): cv.ensure_list(validate_presets),
            cv.Optional(CONF_CUSTOM_PRESETS): cv.ensure_list(validate_custom_presets),
            cv.Optional(CONF_CUSTOM_FAN_MODES): cv.ensure_list(
                validate_custom_fan_modes
            ),
            cv.Optional(CONF_OUTDOOR_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                icon=ICON_THERMOMETER,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_POWER_USAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                icon=ICON_POWER,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_HUMIDITY_SETPOINT): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                icon=ICON_WATER_PERCENT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_HUMIDITY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA),
    cv.only_with_arduino,
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "climate.midea",
    baud_rate=9600,
    require_tx=True,
    require_rx=True,
    data_bits=8,
    parity="NONE",
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    await climate.register_climate(var, config)
    cg.add(var.set_period(config[CONF_PERIOD].total_milliseconds))
    cg.add(var.set_response_timeout(config[CONF_TIMEOUT].total_milliseconds))
    cg.add(var.set_request_attempts(config[CONF_NUM_ATTEMPTS]))
    cg.add(var.set_use_fahrenheit(config[CONF_USE_FAHRENHEIT]))
    if CONF_TRANSMITTER_ID in config:
        cg.add_define("USE_REMOTE_TRANSMITTER")
        # print(config[CONF_TRANSMITTER_ID])
        # transmitter_ = await cg.get_variable(config[CONF_TRANSMITTER_ID])
        # cg.add(var.set_transmitter(transmitter_))
    if CONF_BEEPER in config:
        await new_midea_switch(config[CONF_BEEPER])
    cg.add(var.set_autoconf(config[CONF_AUTOCONF]))
    if CONF_SUPPORTED_MODES in config:
        cg.add(var.set_supported_modes(config[CONF_SUPPORTED_MODES]))
    if CONF_SUPPORTED_SWING_MODES in config:
        cg.add(var.set_supported_swing_modes(config[CONF_SUPPORTED_SWING_MODES]))
    if CONF_SUPPORTED_PRESETS in config:
        cg.add(var.set_supported_presets(config[CONF_SUPPORTED_PRESETS]))
    if CONF_CUSTOM_PRESETS in config:
        cg.add(var.set_custom_presets(config[CONF_CUSTOM_PRESETS]))
    if CONF_CUSTOM_FAN_MODES in config:
        cg.add(var.set_custom_fan_modes(config[CONF_CUSTOM_FAN_MODES]))
    if CONF_OUTDOOR_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_OUTDOOR_TEMPERATURE])
        cg.add(var.set_outdoor_temperature_sensor(sens))
    if CONF_POWER_USAGE in config:
        sens = await sensor.new_sensor(config[CONF_POWER_USAGE])
        cg.add(var.set_power_sensor(sens))
    if CONF_HUMIDITY_SETPOINT in config:
        sens = await sensor.new_sensor(config[CONF_HUMIDITY_SETPOINT])
        cg.add(var.set_humidity_setpoint_sensor(sens))
    cg.add_library("dudanov/MideaUART", "1.1.8")


# AUTOMATIONS

MideaActionBase = midea_ac_ns.class_("MideaActionBase", automation.Action, cg.Parented)
DisplayToggleAction = midea_ac_ns.class_("DisplayToggleAction", MideaActionBase)
BeeperOnAction = midea_ac_ns.class_("BeeperOnAction", MideaActionBase)
BeeperOffAction = midea_ac_ns.class_("BeeperOffAction", MideaActionBase)
PowerOnAction = midea_ac_ns.class_("PowerOnAction", MideaActionBase)
PowerOffAction = midea_ac_ns.class_("PowerOffAction", MideaActionBase)
PowerToggleAction = midea_ac_ns.class_("PowerToggleAction", MideaActionBase)

MIDEA_AC_PARENTED_SCHEMA = midea_parented_schema(AirConditioner)


def register_action(name, type_, schema):
    validator = cv.ensure_schema(schema).extend(MIDEA_AC_PARENTED_SCHEMA)
    registerer = automation.register_action(f"midea_ac.{name}", type_, validator)

    def decorator(func):
        async def new_func(config, action_id, template_arg, args):
            var = cg.new_Pvariable(action_id, template_arg)
            await register_midea(var, config)
            await coroutine(func)(var, config, args)
            return var

        return registerer(new_func)

    return decorator


@register_action("display_toggle", DisplayToggleAction, {})
async def display_toggle_to_code(var, config, args):
    pass


@register_action("beeper_on", BeeperOnAction, {})
async def beeper_on_to_code(var, config, args):
    pass


@register_action("beeper_off", BeeperOffAction, {})
async def beeper_off_to_code(var, config, args):
    pass


@register_action("power_on", PowerOnAction, {})
async def power_on_to_code(var, config, args):
    pass


@register_action("power_off", PowerOffAction, {})
async def power_off_to_code(var, config, args):
    pass


@register_action("power_toggle", PowerToggleAction, {})
async def power_inv_to_code(var, config, args):
    pass
