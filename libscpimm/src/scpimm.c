#include <stdio.h>
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "utils.h"
#include "ieee488.h"
#include "measure.h"
#include "system.h"

/******************************************************************************
  Definitions
******************************************************************************/

#define	VALUE_BUFFER_LENGTH	100

/******************************************************************************
  Forward declarations
******************************************************************************/
static size_t write(scpi_t * context, const char* data, size_t len);
static scpi_result_t reset(scpi_t * context);


/******************************************************************************
  Constant global variables
******************************************************************************/

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
	{"*TST?", SCPI_CoreTstQ},
	{"*WAI", SCPI_CoreWai},

    {"SYSTem:BEEPer", SCPIMM_system_beeper},
    {"SYSTem:BEEPer:STATe", SCPIMM_system_beeper_state},
    {"SYSTem:BEEPer:STATe?", SCPIMM_system_beeper_stateQ},
    {"SYSTem:ERRor?", SCPI_SystemErrorNextQ},
    {"SYSTem:ERRor:NEXT?", SCPI_SystemErrorNextQ},
    {"SYSTem:ERRor:COUNt?", SCPI_SystemErrorCountQ},
    {"SYSTem:LOCal", SCPIMM_system_local},
    {"SYSTem:REMote", SCPIMM_system_remote},
    {"SYSTem:RWLock", SCPIMM_system_rwlock},
    {"SYSTem:VERSion?", SCPI_SystemVersionQ},

	{"STATus:QUEStionable?", SCPI_StatusQuestionableEventQ},
	{"STATus:QUEStionable:EVENt?", SCPI_StatusQuestionableEventQ},
	{"STATus:QUEStionable:ENABle", SCPI_StatusQuestionableEnable},
	{"STATus:QUEStionable:ENABle?", SCPI_StatusQuestionableEnableQ},
	{"STATus:PRESet", SCPI_StatusPreset},

    {"MEASure:VOLTage:DC?", SCPIMM_measure_voltage_dcQ},
    {"MEASure:VOLTage:DC:RATio?", SCPI_StubQ},
    {"MEASure:VOLTage:AC?", SCPI_StubQ},
    {"MEASure:CURRent:DC?", SCPI_StubQ},
    {"MEASure:CURRent:AC?", SCPI_StubQ},
    {"MEASure:RESistance?", SCPI_StubQ},
    {"MEASure:FRESistance?", SCPI_StubQ},
    {"MEASure:FREQuency?", SCPI_StubQ},
    {"MEASure:PERiod?", SCPI_StubQ},

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
	NULL
};

/*static scpi_reg_val_t scpi_regs[SCPI_REG_COUNT];*/

/******************************************************************************
  Global variables
******************************************************************************/

static scpimm_context_t scpimm_context = {
	NULL,
	TRUE,
	SCPIMM_RANGE_DEF,
	SCPIMM_RANGE_DEF,
	SCPIMM_RANGE_DEF,
	SCPIMM_RANGE_DEF
};

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
      0
    },
    &scpi_interface,
    0,
    0,
    false,
    NULL,
    NULL,
    NULL,
    NULL,
	&scpimm_context
};

static float valueBuffer[VALUE_BUFFER_LENGTH];
static size_t valueCounter = 0;

/******************************************************************************
  Interface functions
******************************************************************************/

void SCPIMM_setup(const scpimm_interface_t* i) {
	scpimm_context.interface = (scpimm_interface_t*) i;
	SCPI_Init(&scpi_context);
	reset(&scpi_context);
}

void SCPIMM_parseInBuffer(char const* inbuf, size_t avail) {
	SCPI_Input(&scpi_context, inbuf, avail);
}

void SCPIMM_acceptValue(double v) {
	if (valueCounter < VALUE_BUFFER_LENGTH) {
		valueBuffer[valueCounter++] = (float) v;
	} else {
		// TODO
	}
}

scpimm_context_t* SCPIMM_context() {
	return &scpimm_context;
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
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);

	SCPIMM_set_remote(context, FALSE, FALSE);

	ctx->dcv_range = SCPIMM_RANGE_DEF;
	ctx->dcv_ratio_range = SCPIMM_RANGE_DEF;
	ctx->acv_range = SCPIMM_RANGE_DEF;
	ctx->resistance_range = SCPIMM_RANGE_DEF;

	ctx->interface->set_mode(SCPIMM_MODE_DCV, ctx->dcv_range);

	valueCounter = 0;	

	// TODO

    return SCPI_RES_OK;
}

