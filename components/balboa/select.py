import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import CONF_OPTIONS
from . import balboa_ns
from .const import *
from .switch import BalboaSwitch

BalboaLightSelect = balboa_ns.class_(
    "BalboaLightSelect", select.Select
)

CONF_LIGHT_ID = "light_id"

CONFIG_SCHEMA = (
    select.select_schema(BalboaLightSelect).extend(
        {
            cv.Required(CONF_LIGHT_ID): cv.use_id(BalboaSwitch),
            cv.Optional(CONF_OPTIONS, default=["Red","Green","Blue","Yellow","Cyan","Pink","White","Rainbow"]): cv.All(
                cv.ensure_list(cv.string_strict), cv.Length(min=1)
            )
        }
    )
)

async def to_code(config):
    var = await select.new_select(config, options=config[CONF_OPTIONS])
    light = await cg.get_variable(config[CONF_LIGHT_ID])
    cg.add(var.set_light(light))