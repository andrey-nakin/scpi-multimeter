#include <stdio.h>
#include <string.h>
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "utils.h"
#include "configure.h"
#include "ieee488.h"
#include "measure.h"
#include "system.h"
#include "sense.h"
#include "route.h"
#include "input.h"
#include "dmm.h"
#include "sample.h"
#include "trigger.h"
#include "display.h"

/******************************************************************************
  Definitions
******************************************************************************/

#define SCPIMM_VERSION "1992.0"

/******************************************************************************
  Forward declarations
******************************************************************************/
static size_t write(scpi_t * context, const char* data, size_t len);
static scpi_result_t reset(scpi_t * context);
static scpi_result_t test(scpi_t * context);
static scpi_result_t system_versionQ(scpi_t* context);

/******************************************************************************
  Constant global variables
******************************************************************************/

#define DECL_NO_PARAM_FUNCTION(func_name, routine_name)	\
	{"CONFigure:" func_name, SCPIMM_configure_ ## routine_name},	\
	{"MEASure:" func_name "?", SCPIMM_measure_ ## routine_name ## Q},

#define DECL_GENERIC_AC_FUNCTION(func_name, routine_name)	\
	DECL_NO_PARAM_FUNCTION(func_name, routine_name)	\
	{"SENSe:" func_name ":RANGe", SCPIMM_sense_ ## routine_name ## _range},	\
	{"SENSe:" func_name ":RANGe?", SCPIMM_sense_ ## routine_name ## _rangeQ},	\
	{"SENSe:" func_name ":RANGe:AUTO", SCPIMM_sense_ ## routine_name ## _range_auto},	\
	{"SENSe:" func_name ":RANGe:AUTO?", SCPIMM_sense_ ## routine_name ## _range_autoQ},	\
	{"SENSe:" func_name ":RESolution", SCPIMM_sense_ ## routine_name ## _resolution},	\
	{"SENSe:" func_name ":RESolution?", SCPIMM_sense_ ## routine_name ## _resolutionQ},	\
	{func_name ":RANGe", SCPIMM_sense_ ## routine_name ## _range},	\
	{func_name ":RANGe?", SCPIMM_sense_ ## routine_name ## _rangeQ},	\
	{func_name ":RANGe:AUTO", SCPIMM_sense_ ## routine_name ## _range_auto},	\
	{func_name ":RANGe:AUTO?", SCPIMM_sense_ ## routine_name ## _range_autoQ},	\
	{func_name ":RESolution", SCPIMM_sense_ ## routine_name ## _resolution},	\
	{func_name ":RESolution?", SCPIMM_sense_ ## routine_name ## _resolutionQ},

#define DECL_GENERIC_DC_FUNCTION(func_name, routine_name)	\
	DECL_GENERIC_AC_FUNCTION(func_name, routine_name)	\
	{"SENSe:" func_name ":NPLCycles", SCPIMM_sense_ ## routine_name ## _nplcycles},	\
	{"SENSe:" func_name ":NPLCycles?", SCPIMM_sense_ ## routine_name ## _nplcyclesQ},	\
	{func_name ":NPLCycles", SCPIMM_sense_ ## routine_name ## _nplcycles},	\
	{func_name ":NPLCycles?", SCPIMM_sense_ ## routine_name ## _nplcyclesQ},

static const scpi_command_t scpi_commands[] = {
	{"*CLS", SCPI_CoreCls},
	{"*ESE", SCPI_CoreEse},
	{"*ESE?", SCPI_CoreEseQ},
	{"*ESR?", SCPI_CoreEsrQ},
	{"*IDN?", SCPIMM_IdnQ},
	{"*OPC", SCPI_CoreOpc},
	{"*OPC?", SCPI_CoreOpcQ},
	{"*RST", SCPI_CoreRst},
	{"*SRE", SCPI_CoreSre},
	{"*SRE?", SCPI_CoreSreQ},
	{"*STB?", SCPI_CoreStbQ},
	{"*TRG", SCPIMM_trg},
	{"*TST?", SCPI_CoreTstQ},
	{"*WAI", SCPI_CoreWai},

    {"FETCh?", SCPIMM_fetchQ},
    {"INITiate", SCPIMM_initiate},
    {"READ?", SCPIMM_readQ},
    {"DATA:POINts?", SCPIMM_data_pointsQ},

    {"CONFigure?", SCPIMM_configureQ},

    /* declare generic CONFIGURE:* and SENSE:* commands */

#ifndef SCPIMM_NO_VOLTAGE_DC
    DECL_GENERIC_DC_FUNCTION("VOLTage", voltage_dc)
    DECL_GENERIC_DC_FUNCTION("VOLTage:DC", voltage_dc)
#endif

#ifndef SCPIMM_NO_VOLTAGE_DC_RATIO
    DECL_NO_PARAM_FUNCTION("VOLTage:DC:RATio", voltage_dc_ratio)
#endif

#ifndef SCPIMM_NO_VOLTAGE_AC
    DECL_GENERIC_AC_FUNCTION("VOLTage:AC", voltage_ac)
#endif

#ifndef SCPIMM_NO_VOLTAGE_AC_RATIO
    DECL_NO_PARAM_FUNCTION("VOLTage:AC:RATio", voltage_ac_ratio)
#endif

#ifndef SCPIMM_NO_CURRENT_DC
    DECL_GENERIC_DC_FUNCTION("CURRent", current_dc)
    DECL_GENERIC_DC_FUNCTION("CURRent:DC", current_dc)
#endif

#ifndef SCPIMM_NO_CURRENT_AC
    DECL_GENERIC_AC_FUNCTION("CURRent:AC", current_ac)
#endif

#ifndef SCPIMM_NO_RESISTANCE
    DECL_GENERIC_DC_FUNCTION("RESistance", resistance)
#endif

#ifndef SCPIMM_NO_FRESISTANCE
    DECL_GENERIC_DC_FUNCTION("FRESistance", fresistance)
#endif

    /* declare specific CONFIGURE:* and SENSE:* commands */

	{"SENSe:FUNCtion", SCPIMM_sense_function},
	{"SENSe:FUNCtion?", SCPIMM_sense_functionQ},
	{"SENSe:ZERO:AUTO", SCPIMM_sense_zero_auto},
	{"SENSe:ZERO:AUTO?", SCPIMM_sense_zero_autoQ},

	{"FUNCtion", SCPIMM_sense_function},
	{"FUNCtion?", SCPIMM_sense_functionQ},
	{"ZERO:AUTO", SCPIMM_sense_zero_auto},
	{"ZERO:AUTO?", SCPIMM_sense_zero_autoQ},

	{"SAMPle:COUNt", SCPIMM_sample_count},
	{"SAMPle:COUNt?", SCPIMM_sample_countQ},

    {"DISPlay", SCPIMM_display},
    {"DISPlay?", SCPIMM_displayQ},
    {"DISPlay:TEXT", SCPIMM_display_text},
    {"DISPlay:TEXT?", SCPIMM_display_textQ},
    {"DISPlay:TEXT:CLEar", SCPIMM_display_text_clear},

    {"INPut:IMPedance:AUTO", SCPIMM_input_impedance_auto},
    {"INPut:IMPedance:AUTO?", SCPIMM_input_impedance_autoQ},

    {"SYSTem:BEEPer", SCPIMM_system_beeper},
    {"SYSTem:BEEPer:STATe", SCPIMM_system_beeper_state},
    {"SYSTem:BEEPer:STATe?", SCPIMM_system_beeper_stateQ},
    {"SYSTem:ERRor?", SCPI_SystemErrorNextQ},
    {"SYSTem:ERRor:NEXT?", SCPI_SystemErrorNextQ},
    {"SYSTem:ERRor:COUNt?", SCPI_SystemErrorCountQ},
    {"SYSTem:LOCal", SCPIMM_system_local},
    {"SYSTem:REMote", SCPIMM_system_remote},
    {"SYSTem:RWLock", SCPIMM_system_rwlock},
    {"SYSTem:VERSion?", system_versionQ},

	{"STATus:QUEStionable?", SCPI_StatusQuestionableEventQ},
	{"STATus:QUEStionable:EVENt?", SCPI_StatusQuestionableEventQ},
	{"STATus:QUEStionable:ENABle", SCPI_StatusQuestionableEnable},
	{"STATus:QUEStionable:ENABle?", SCPI_StatusQuestionableEnableQ},
	{"STATus:PRESet", SCPI_StatusPreset},

	{"ROUTe:TERMinals?", SCPIMM_route_terminals},

	{"TRIGger:SOURce", SCPIMM_trigger_source},
	{"TRIGger:SOURce?", SCPIMM_trigger_sourceQ},
	{"TRIGger:DELay", SCPIMM_trigger_delay},
	{"TRIGger:DELay?", SCPIMM_trigger_delayQ},
	{"TRIGger:DELay:AUTO", SCPIMM_trigger_delay_auto},
	{"TRIGger:DELay:AUTO?", SCPIMM_trigger_delay_autoQ},
	{"TRIGger:COUNt", SCPIMM_trigger_count},
	{"TRIGger:COUNt?", SCPIMM_trigger_countQ},

	SCPI_CMD_LIST_END
};

#define SCPI_INPUT_BUFFER_LENGTH 256
static char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];

static scpi_interface_t scpi_interface = {
    NULL,
    write,
	NULL,
	NULL,
	reset,
	test
};

/*static scpi_reg_val_t scpi_regs[SCPI_REG_COUNT];*/

/******************************************************************************
  Global variables
******************************************************************************/

static volatile scpimm_context_t scpimm_context;

static scpi_t scpi_context = {
    scpi_commands,
    {
        SCPI_INPUT_BUFFER_LENGTH,
        0,
        scpi_input_buffer,
    },
    {
      NULL,
      NULL,
      0,
      {
        0,
        0,
        NULL
      }
    },
    &scpi_interface,
    0,
    0,
    false,
    NULL,
    NULL,
    scpi_units_def,
    scpi_special_numbers_def,
	(void*) &scpimm_context,
	{NULL, NULL, NULL, NULL}
};

/******************************************************************************
  Interface functions
******************************************************************************/

void SCPIMM_setup(const scpimm_interface_t* i) {
	scpimm_context.interface = (scpimm_interface_t*) i;
	SCPI_Init(&scpi_context);
	i->setup();
	reset(&scpi_context);
}

void SCPIMM_parse_in_buffer(char const* inbuf, size_t avail) {
	SCPI_Input(&scpi_context, inbuf, avail);
}

scpimm_context_t* SCPIMM_context() {
	return (scpimm_context_t*) &scpimm_context;
}

scpi_t* SCPI_context() {
	return &scpi_context;
}

/******************************************************************************
  Internals
******************************************************************************/

static size_t write(scpi_t* const context, const char* data, size_t len) {
	return SCPIMM_INTERFACE(context)->send((const uint8_t*) data, len);
}

static scpi_result_t reset(scpi_t* context) {
	volatile scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);

//	memset((void*) ctx, 0, sizeof(*ctx));
	SCPIMM_set_remote(context, TRUE, FALSE);
	ctx->display = TRUE;
	ctx->display_text[0] = '\0';

	if (ctx->interface->reset) {
		ctx->interface->reset();
	}

	{
		scpi_number_t def = {0.0, SCPI_UNIT_NONE, SCPI_NUM_DEF};
		SCPIMM_do_configure(context, SCPIMM_MODE_DCV, &def, &def);
	}

	// TODO

    return SCPI_RES_OK;
}

static scpi_result_t test(scpi_t* context) {
	scpimm_interface_t* const intf = SCPIMM_INTERFACE(context);

	if (intf->test) {
		int16_t err;

		if (SCPI_ERROR_OK != (err = intf->test())) {
			SCPI_ErrorPush(context, err);
			return (scpi_result_t) 1;
		}
	}

	return (scpi_result_t) 0;
}

static scpi_result_t system_versionQ(scpi_t* context) {
	SCPI_ResultString(context, SCPIMM_VERSION);
	return SCPI_RES_OK;
}

