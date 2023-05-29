import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, time, globals
from esphome.const import CONF_ID, CONF_TIME_ID, CONF_UART_ID
from .const import CONF_SERIALENABLED

DEPENDENCIES = ["uart"]
MULTI_CONF = True

balboa_ns = cg.esphome_ns.namespace("balboa")

BalboaComponent = balboa_ns.class_(
    "BalboaComponent", cg.Component
)

CONF_BALBOA_ID = "balboa_id"

BALBOA_COMPONENT_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_BALBOA_ID): cv.use_id(BalboaComponent)
    }
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BalboaComponent),
            cv.Optional(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
            cv.Optional(CONF_SERIALENABLED): cv.use_id(globals.GlobalsComponent)
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_id(config[CONF_ID].__str__()))

    if CONF_TIME_ID in config:
        time_ = await cg.get_variable(config[CONF_TIME_ID])
        cg.add(var.set_rtc(time_))

    if CONF_SERIALENABLED in config:
        serialenabled = await cg.get_variable(config[CONF_SERIALENABLED])
        cg.add(var.set_serialenabled_var(serialenabled))