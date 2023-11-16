import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.core import coroutine
from esphome.automation import Action, register_action
from esphome.components.remote_base import (
    MideaData,
    register_transmittable,
    REMOTE_TRANSMITTABLE_SCHEMA,
    RemoteTransmittable,
)
from esphome.const import (
    CONF_BEEPER,
    CONF_TEMPERATURE,
    CONF_USE_FAHRENHEIT,
)

CODEOWNERS = ["@dudanov"]
AUTO_LOAD = ["remote_base"]

CONF_DISPLAY_TOGGLE = "display_toggle"
CONF_FOLLOW_ME = "follow_me"
CONF_VSWING_STEP = "vswing_step"

midea_ir_ns = cg.esphome_ns.namespace("midea_ir")

# SPECIAL COMMANDS

SpecialData = midea_ir_ns.class_("SpecialData", MideaData)
SpecialCommand = midea_ir_ns.enum("SpecialCommand")

SPECIAL_COMMANDS = {
    "VSWING_STEP": SpecialCommand.VSWING_STEP,
    "DISPLAY_TOGGLE": SpecialCommand.DISPLAY_TOGGLE,
    "VSWING_TOGGLE": SpecialCommand.VSWING_TOGGLE,
    "TURBO_TOGGLE": SpecialCommand.TURBO_TOGGLE,
}

VerticalSwingStepData = midea_ir_ns.class_("VerticalSwingStepData", SpecialData)
VerticalSwingToggleData = midea_ir_ns.class_("VerticalSwingToggleData", SpecialData)
DisplayToggleData = midea_ir_ns.class_("DisplayToggleData", SpecialData)
TurboToggleData = midea_ir_ns.class_("TurboToggleData", SpecialData)

# ACTIONS

MideaRemoteAction = midea_ir_ns.class_("MideaRemoteAction", Action, RemoteTransmittable)

DisplayToggleAction = midea_ir_ns.class_("DisplayToggleAction", MideaRemoteAction)
FollowMeAction = midea_ir_ns.class_("FollowMeAction", MideaRemoteAction)
SwingStepAction = midea_ir_ns.class_("SwingStepAction", MideaRemoteAction)


def register_remote_action(name, type_, schema):
    validator = cv.ensure_schema(schema).extend(REMOTE_TRANSMITTABLE_SCHEMA)
    registerer = register_action(f"midea_ir.{name}", type_, validator)

    def decorator(func):
        async def new_func(config, action_id, template_arg, args):
            var = cg.new_Pvariable(action_id, template_arg)
            await register_transmittable(var, config)
            await coroutine(func)(var, config, args)
            return var

        return registerer(new_func)

    return decorator


FOLLOW_ME_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_TEMPERATURE): cv.templatable(cv.temperature),
        cv.Optional(CONF_USE_FAHRENHEIT, default=False): cv.templatable(cv.boolean),
        cv.Optional(CONF_BEEPER, default=False): cv.templatable(cv.boolean),
    }
)


@register_remote_action(CONF_FOLLOW_ME, FollowMeAction, FOLLOW_ME_SCHEMA)
async def follow_me_to_code(var, config, args):
    template_ = await cg.templatable(config[CONF_TEMPERATURE], args, cg.float_)
    cg.add(var.set_temperature(template_))
    template_ = await cg.templatable(config[CONF_USE_FAHRENHEIT], args, cg.bool_)
    cg.add(var.set_use_fahrenheit(template_))
    template_ = await cg.templatable(config[CONF_BEEPER], args, cg.bool_)
    cg.add(var.set_beeper(template_))


# Toggle Display action
@register_remote_action(CONF_DISPLAY_TOGGLE, DisplayToggleAction, {})
async def display_toggle_to_code(var, config, args):
    pass


# Swing Step action
@register_remote_action(CONF_VSWING_STEP, SwingStepAction, {})
async def swing_step_to_code(var, config, args):
    pass


CONFIG_SCHEMA = {}
