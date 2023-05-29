import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from . import CONF_BALBOA_ID, BALBOA_COMPONENT_SCHEMA
from .const import *

CONFIG_SCHEMA = (
    BALBOA_COMPONENT_SCHEMA.extend(
        {
            cv.Optional(CONF_CIRC): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_HEATING): binary_sensor.binary_sensor_schema(),
        }
    )
)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_BALBOA_ID])

    if CONF_CIRC in config:
        conf = config[CONF_CIRC]
        var = await binary_sensor.new_binary_sensor(conf)
        cg.add(parent.set_circ_sensor(var))
    if CONF_HEATING in config:
        conf = config[CONF_HEATING]
        var = await binary_sensor.new_binary_sensor(conf)
        cg.add(parent.set_heating_sensor(var))