import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome import pins
from esphome.components import output
from esphome.components import number
from esphome.components import switch
from esphome.components import text_sensor
from esphome.const import CONF_ID, CONF_PIN

CONF_BOILER_ID     = "boiler_id"
INIT_DELAI         = "init_delai"
REPEAT_DELAI       = "repeat_delai"
NEW_SETPOINT_DELAI = "new_setpoint_delai"
TIME_TO_DEATH      = "time_to_death"
MAX_SETPOINT       = "max_setpoint"
STATE_CONTROL      = "state_control"
FIRMWARE_VERSION   = "firmware_version_sensor"
FIRMWARE_VERSION_SENSOR = "firmware_version"

frisquet_boiler = cg.esphome_ns.namespace('frisquet_boiler')
FrisquetBoiler = frisquet_boiler.class_('FrisquetBoiler', output.FloatOutput,cg.Component)

# Actions
SetModeAction = frisquet_boiler.class_("SetModeAction", automation.Action)

CONFIG_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend({
    cv.Required(CONF_ID): cv.declare_id(FrisquetBoiler),
    cv.Required(CONF_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_BOILER_ID): cv.string,
    cv.Optional(INIT_DELAI, 120): cv.int_,
    cv.Optional(REPEAT_DELAI, 300): cv.int_,
    cv.Optional(NEW_SETPOINT_DELAI, 5): cv.int_,
    cv.Optional(TIME_TO_DEATH, 1200): cv.int_,
    cv.Optional(MAX_SETPOINT): cv.use_id(number.Number),
    cv.Optional(STATE_CONTROL): cv.use_id(switch.Switch),
    cv.Required(FIRMWARE_VERSION): cv.use_id(text_sensor.TextSensor),
    cv.Required(FIRMWARE_VERSION_SENSOR): cv.string,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await output.register_output(var, config)
    await cg.register_component(var, config)

    boiler_id = config[CONF_BOILER_ID]
    cg.add(var.set_boiler_id(boiler_id))

    pin = await cg.gpio_pin_expression(config[CONF_PIN])
    cg.add(var.set_pin(pin))

    value = config[INIT_DELAI]
    cg.add(var.set_init_delai(value))

    value = config[REPEAT_DELAI]
    cg.add(var.set_repeat_delai(value))

    value = config[NEW_SETPOINT_DELAI]
    cg.add(var.set_new_setpoint_delai(value))

    value = config[TIME_TO_DEATH]
    cg.add(var.set_time_to_death(value))

    value = config[MAX_SETPOINT]
    ref = await cg.get_variable(value)
    cg.add(var.set_max_setpoint_reference(ref))

    value = config[STATE_CONTROL]
    ref = await cg.get_variable(value)
    cg.add(var.set_state_control_reference(ref))

    value = config[FIRMWARE_VERSION]
    ref = await cg.get_variable(value)
    cg.add(var.set_firmware_version_reference(ref))
    cg.add(var.set_firmware_version(config[FIRMWARE_VERSION_SENSOR]))
