import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components.sensor import new_sensor, sensor_schema
from esphome.components.remote_base import (
    RemoteReceiverListener,
    register_listener,
    REMOTE_LISTENER_SCHEMA,
)
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)
from esphome.components.midea_ir import CONF_FOLLOW_ME, midea_ir_ns

DEPENDENCIES = ["remote_receiver"]
AUTO_LOAD = ["midea_ir", "sensor"]


MideaRemoteSensors = midea_ir_ns.class_("MideaRemoteSensors", RemoteReceiverListener)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(MideaRemoteSensors),
        cv.Optional(CONF_FOLLOW_ME): sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            icon="mdi:home-thermometer-outline",
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    }
).extend(REMOTE_LISTENER_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await register_listener(var, config)
    if CONF_FOLLOW_ME in config:
        sensor_ = await new_sensor(config[CONF_FOLLOW_ME])
        cg.add(var.set_fm_sensor(sensor_))
