import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from . import CONF_BALBOA_ID, BALBOA_COMPONENT_SCHEMA, balboa_ns
from .const import *

BalboaSwitch = balboa_ns.class_(
    "BalboaSwitch", switch.Switch
)

def balboa_switch_schema(item_id: int):
    return switch.switch_schema(BalboaSwitch).extend({
            cv.Optional(CONF_ITEM_ID, default=item_id): cv.hex_int_range(min=0,max=255)
        })

CONFIG_SCHEMA = (
    BALBOA_COMPONENT_SCHEMA.extend(
        {
            cv.Optional(CONF_HIGHRANGE): balboa_switch_schema(CONF_HIGHRANGE_ITEM),
            cv.Optional(CONF_LIGHT1): balboa_switch_schema(CONF_LIGHT1_ITEM),
            cv.Optional(CONF_PUMP1): balboa_switch_schema(CONF_PUMP1_ITEM),
            cv.Optional(CONF_PUMP2): balboa_switch_schema(CONF_PUMP2_ITEM),
        }
    )
)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_BALBOA_ID])

    if CONF_HIGHRANGE in config:
        conf = config[CONF_HIGHRANGE]
        var = await switch.new_switch(conf)
        cg.add(var.set_item(conf[CONF_ITEM_ID]))
        cg.add(var.set_parent(parent))
        cg.add(parent.set_highrange_switch(var))
    if CONF_LIGHT1 in config:
        conf = config[CONF_LIGHT1]
        var = await switch.new_switch(conf)
        cg.add(var.set_item(conf[CONF_ITEM_ID]))
        cg.add(var.set_parent(parent))
        cg.add(parent.set_light1_switch(var))
    if CONF_PUMP1 in config:
        conf = config[CONF_PUMP1]
        var = await switch.new_switch(conf)
        cg.add(var.set_item(conf[CONF_ITEM_ID]))
        cg.add(var.set_parent(parent))
        cg.add(parent.set_pump1_switch(var))
    if CONF_PUMP2 in config:
        conf = config[CONF_PUMP2]
        var = await switch.new_switch(conf)
        cg.add(var.set_item(conf[CONF_ITEM_ID]))
        cg.add(var.set_parent(parent))
        cg.add(parent.set_pump2_switch(var))