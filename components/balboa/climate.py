import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from esphome.const import CONF_ID
from . import CONF_BALBOA_ID, BALBOA_COMPONENT_SCHEMA, balboa_ns
from .const import *

BalboaClimate = balboa_ns.class_(
    "BalboaClimate", climate.Climate
)

CONFIG_SCHEMA = (
    BALBOA_COMPONENT_SCHEMA.extend(
        {
            cv.Optional(CONF_THERMOSTAT): climate.CLIMATE_SCHEMA.extend({cv.GenerateID(): cv.declare_id(BalboaClimate)})
        }
    )
)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_BALBOA_ID])

    if CONF_THERMOSTAT in config:
        conf = config[CONF_THERMOSTAT]
        var = cg.new_Pvariable(conf[CONF_ID])
        await climate.register_climate(var, conf)
        cg.add(var.set_parent(parent))
        cg.add(parent.set_thermostat(var))