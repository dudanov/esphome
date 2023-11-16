import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.cpp_helpers import register_parented

CODEOWNERS = ["@dudanov"]
CONF_MIDEA_ID = "midea_id"

midea_ac_ns = cg.esphome_ns.namespace("midea").namespace("ac")


def register_midea(var, config):
    return register_parented(var, config[CONF_MIDEA_ID])


def midea_parented_schema(class_):
    return cv.Schema(
        {
            cv.GenerateID(CONF_MIDEA_ID): cv.use_id(class_),
        }
    )
