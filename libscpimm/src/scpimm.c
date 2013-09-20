#include <stdio.h>
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "ieee488.h"
#include "measure.h"

/* forward declarations */
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

    {"SYSTem:ERRor?", SCPI_SystemErrorNextQ},
    {"SYSTem:ERRor:NEXT?", SCPI_SystemErrorNextQ},
    {"SYSTem:ERRor:COUNt?", SCPI_SystemErrorCountQ},
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
	reset
};

//static scpi_reg_val_t scpi_regs[SCPI_REG_COUNT];

scpi_t scpi_context = {
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
    &scpi_interface
/*    0,
    0,
    0,
    NULL,
    scpi_regs,
    scpi_units_def,
    scpi_special_numbers_def, */
};

/******************************************************************************
  Global variables
******************************************************************************/

static float dcvRange;
static float dcvRatioRange;
static float acvRange;
static float resistanceRange;

/******************************************************************************
  Interface functions
******************************************************************************/

void SCPIMM_setup() {
	SCPI_Init(&scpi_context);

	dcvRange = SCPIMM_RANGE_DEF;
	dcvRatioRange = SCPIMM_RANGE_DEF;
	acvRange = SCPIMM_RANGE_DEF;
	resistanceRange = SCPIMM_RANGE_DEF;

	SCPIMM_setMode(SCPIMM_MODE_DCV);
	SCPIMM_setDCVRange(dcvRange);

	// TODO
}

void SCPIMM_parseInBuffer(char const* inbuf, size_t avail) {
	SCPI_Input(&scpi_context, inbuf, avail);
}

void SCPIMM_acceptValue(double) {
	// TODO
}

/******************************************************************************
  Internals
******************************************************************************/

static size_t write(scpi_t* const context, const char* data, size_t len) {
	(void) context;
	return SCPIMM_send((const uint8_t*) data, len);
}

static scpi_result_t reset(scpi_t* const context) {
	(void) context;
	SCPIMM_setup();
    return SCPI_RES_OK;
}

