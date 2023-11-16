import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components.button import (
    Button,
    button_schema,
    new_button,
)
from esphome.components.remote_base import (
    register_transmittable,
    RemoteTransmittable,
    REMOTE_TRANSMITTABLE_SCHEMA,
)
from esphome.components.midea_ir import (
    midea_ir_ns,
    CONF_VSWING_STEP,
    CONF_DISPLAY_TOGGLE,
    SpecialCommand,
    SPECIAL_COMMANDS,
)
from esphome.const import CONF_COMMAND

DEPENDENCIES = ["remote_transmitter"]
AUTO_LOAD = ["midea_ir", "button"]

CONF_SPECIAL = "special"

SpecialButton = midea_ir_ns.class_("SpecialButton", Button, RemoteTransmittable)


def special_schema(icon: str):
    return button_schema(SpecialButton, icon=icon).extend(REMOTE_TRANSMITTABLE_SCHEMA)


CONFIG_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_VSWING_STEP): special_schema("mdi:tailwind"),
        cv.Optional(CONF_DISPLAY_TOGGLE): special_schema("mdi:theme-light-dark"),
        cv.Optional(CONF_SPECIAL): cv.All(
            cv.ensure_list(
                cv.Schema(
                    {
                        cv.Required(CONF_COMMAND): cv.Any(
                            cv.uint8_t, cv.enum(SPECIAL_COMMANDS)
                        ),
                    }
                ).extend(special_schema("mdi:remote"))
            ),
            cv.Length(min=1),
        ),
    }
)


async def new_special(config, command):
    var = await new_button(config)
    await register_transmittable(var, config)
    cg.add(var.set_command(command))


async def to_code(config):
    if CONF_VSWING_STEP in config:
        await new_special(config[CONF_VSWING_STEP], SpecialCommand.VSWING_STEP)
    if CONF_DISPLAY_TOGGLE in config:
        await new_special(config[CONF_DISPLAY_TOGGLE], SpecialCommand.DISPLAY_TOGGLE)
    if CONF_SPECIAL in config:
        for btn in config[CONF_SPECIAL]:
            await new_special(btn, btn[CONF_COMMAND])
