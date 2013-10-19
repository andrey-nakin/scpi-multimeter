#include <stdio.h>
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

/******************************************************************************
  Definitions
******************************************************************************/

#define SCPIMM_VERSION "1992.0"

/******************************************************************************
  Forward declarations
******************************************************************************/
static size_t write(scpi_t * context, const char* data, size_t len);
static scpi_result_t reset(scpi_t * context);
static scpi_result_t system_versionQ(scpi_t* context);

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
	{"*TRG", SCPIMM_trg},
	{"*TST?", SCPI_CoreTstQ},
	{"*WAI", SCPI_CoreWai},

    {"FETCh?", SCPIMM_fetchQ},
    {"INITiate", SCPIMM_initiate},
    {"READ?", SCPIMM_readQ},

    {"CONFigure?", SCPIMM_configureQ},
    {"CONFigure:VOLTage:DC", SCPIMM_configure_voltage_dc},
    {"CONFigure:VOLTage:DC:RATio", SCPIMM_configure_voltage_dc_ratio},
    {"CONFigure:VOLTage:AC", SCPIMM_configure_voltage_ac},
    {"CONFigure:CURRent:DC", SCPIMM_configure_current_dc},
    {"CONFigure:CURRent:AC", SCPIMM_configure_current_ac},
    {"CONFigure:RESistance", SCPIMM_configure_resistance},
    {"CONFigure:FRESistance", SCPIMM_configure_fresistance},
    {"CONFigure:FREQuency", SCPIMM_configure_frequency},
    {"CONFigure:PERiod", SCPIMM_configure_period},
    {"CONFigure:CONTinuity", SCPIMM_configure_continuity},
    {"CONFigure:DIODe", SCPIMM_configure_diode},

    {"INPut:IMPedance:AUTO", SCPIMM_input_impedance_auto},
    {"INPut:IMPedance:AUTO?", SCPIMM_input_impedance_autoQ},

    {"MEASure:VOLTage:DC?", SCPIMM_measure_voltage_dcQ},
    {"MEASure:VOLTage:DC:RATio?", SCPIMM_measure_voltage_dc_ratioQ},
    {"MEASure:VOLTage:AC?", SCPIMM_measure_voltage_acQ},
    {"MEASure:CURRent:DC?", SCPIMM_measure_current_dcQ},
    {"MEASure:CURRent:AC?", SCPIMM_measure_current_acQ},
    {"MEASure:RESistance?", SCPIMM_measure_resistanceQ},
    {"MEASure:FRESistance?", SCPIMM_measure_fresistanceQ},
    {"MEASure:FREQuency?", SCPIMM_measure_frequencyQ},
    {"MEASure:PERiod?", SCPIMM_measure_periodQ},
    {"MEASure:CONTinuity?", SCPIMM_measure_continuityQ},
    {"MEASure:DIODe?", SCPIMM_measure_diodeQ},

	{"SENSe:FUNCtion", SCPIMM_sense_function},
	{"SENSe:FUNCtion?", SCPIMM_sense_functionQ},
	{"SENSe:VOLTage:DC:RANGe", SCPIMM_sense_voltage_dc_range},
	{"SENSe:VOLTage:DC:RANGe?", SCPIMM_sense_voltage_dc_rangeQ},
	{"SENSe:VOLTage:AC:RANGe", SCPIMM_sense_voltage_ac_range},
	{"SENSe:VOLTage:AC:RANGe?", SCPIMM_sense_voltage_ac_rangeQ},
	{"SENSe:CURRent:DC:RANGe", SCPIMM_sense_current_dc_range},
	{"SENSe:CURRent:DC:RANGe?", SCPIMM_sense_current_dc_rangeQ},
	{"SENSe:CURRent:AC:RANGe", SCPIMM_sense_current_ac_range},
	{"SENSe:CURRent:AC:RANGe?", SCPIMM_sense_current_ac_rangeQ},
	{"SENSe:RESistance:RANGe", SCPIMM_sense_resistance_range},
	{"SENSe:RESistance:RANGe?", SCPIMM_sense_resistance_rangeQ},
	{"SENSe:FRESistance:RANGe", SCPIMM_sense_fresistance_range},
	{"SENSe:FRESistance:RANGe?", SCPIMM_sense_fresistance_rangeQ},
	{"SENSe:FREQuency:VOLTage:RANGe", SCPIMM_sense_frequency_voltage_range},
	{"SENSe:FREQuency:VOLTage:RANGe?", SCPIMM_sense_frequency_voltage_rangeQ},
	{"SENSe:PERiod:VOLTage:RANGe", SCPIMM_sense_period_voltage_range},
	{"SENSe:PERiod:VOLTage:RANGe?", SCPIMM_sense_period_voltage_rangeQ},
	{"SENSe:VOLTage:DC:RANGe:AUTO", SCPIMM_sense_voltage_dc_range_auto},
	{"SENSe:VOLTage:DC:RANGe:AUTO?", SCPIMM_sense_voltage_dc_range_autoQ},
	{"SENSe:VOLTage:AC:RANGe:AUTO", SCPIMM_sense_voltage_ac_range_auto},
	{"SENSe:VOLTage:AC:RANGe:AUTO?", SCPIMM_sense_voltage_ac_range_autoQ},
	{"SENSe:CURRent:DC:RANGe:AUTO", SCPIMM_sense_current_dc_range_auto},
	{"SENSe:CURRent:DC:RANGe:AUTO?", SCPIMM_sense_current_dc_range_autoQ},
	{"SENSe:CURRent:AC:RANGe:AUTO", SCPIMM_sense_current_ac_range_auto},
	{"SENSe:CURRent:AC:RANGe:AUTO?", SCPIMM_sense_current_ac_range_autoQ},
	{"SENSe:RESistance:RANGe:AUTO", SCPIMM_sense_resistance_range_auto},
	{"SENSe:RESistance:RANGe:AUTO?", SCPIMM_sense_resistance_range_autoQ},
	{"SENSe:FRESistance:RANGe:AUTO", SCPIMM_sense_fresistance_range_auto},
	{"SENSe:FRESistance:RANGe:AUTO?", SCPIMM_sense_fresistance_range_autoQ},
	{"SENSe:FREQuency:VOLTage:RANGe:AUTO", SCPIMM_sense_frequency_voltage_range_auto},
	{"SENSe:FREQuency:VOLTage:RANGe:AUTO?", SCPIMM_sense_frequency_voltage_range_autoQ},
	{"SENSe:PERiod:VOLTage:RANGe:AUTO", SCPIMM_sense_period_voltage_range_auto},
	{"SENSe:PERiod:VOLTage:RANGe:AUTO?", SCPIMM_sense_period_voltage_range_autoQ},
	{"SENSe:VOLTage:DC:RESolution", SCPIMM_sense_voltage_dc_resolution},
	{"SENSe:VOLTage:DC:RESolution?", SCPIMM_sense_voltage_dc_resolutionQ},
	{"SENSe:VOLTage:AC:RESolution", SCPIMM_sense_voltage_ac_resolution},
	{"SENSe:VOLTage:AC:RESolution?", SCPIMM_sense_voltage_ac_resolutionQ},
	{"SENSe:CURRent:DC:RESolution", SCPIMM_sense_current_dc_resolution},
	{"SENSe:CURRent:DC:RESolution?", SCPIMM_sense_current_dc_resolutionQ},
	{"SENSe:CURRent:AC:RESolution", SCPIMM_sense_current_ac_resolution},
	{"SENSe:CURRent:AC:RESolution?", SCPIMM_sense_current_ac_resolutionQ},
	{"SENSe:RESistance:RESolution", SCPIMM_sense_resistance_resolution},
	{"SENSe:RESistance:RESolution?", SCPIMM_sense_resistance_resolutionQ},
	{"SENSe:FRESistance:RESolution", SCPIMM_sense_fresistance_resolution},
	{"SENSe:FRESistance:RESolution?", SCPIMM_sense_fresistance_resolutionQ},
	{"SENSe:VOLTage:DC:NPLCycles", SCPIMM_sense_voltage_dc_nplcycles},
	{"SENSe:VOLTage:DC:NPLCycles?", SCPIMM_sense_voltage_dc_nplcyclesQ},
	{"SENSe:CURRent:DC:NPLCycles", SCPIMM_sense_current_dc_nplcycles},
	{"SENSe:CURRent:DC:NPLCycles?", SCPIMM_sense_current_dc_nplcyclesQ},
	{"SENSe:RESistance:NPLCycles", SCPIMM_sense_resistance_nplcycles},
	{"SENSe:RESistance:NPLCycles?", SCPIMM_sense_resistance_nplcyclesQ},
	{"SENSe:FRESistance:NPLCycles", SCPIMM_sense_fresistance_nplcycles},
	{"SENSe:FRESistance:NPLCycles?", SCPIMM_sense_fresistance_nplcyclesQ},
	{"SENSe:FREQuency:APERture", SCPIMM_sense_frequency_aperture},
	{"SENSe:FREQuency:APERture?", SCPIMM_sense_frequency_apertureQ},
	{"SENSe:PERiod:APERture", SCPIMM_sense_period_aperture},
	{"SENSe:PERiod:APERture?", SCPIMM_sense_period_apertureQ},
	{"SENSe:DETector:BANDwidth", SCPIMM_sense_detector_bandwidth},
	{"SENSe:DETector:BANDwidth?", SCPIMM_sense_detector_bandwidthQ},
	{"SENSe:ZERO:AUTO", SCPIMM_sense_zero_auto},
	{"SENSe:ZERO:AUTO?", SCPIMM_sense_zero_autoQ},

	{"FUNCtion", SCPIMM_sense_function},
	{"FUNCtion?", SCPIMM_sense_functionQ},
	{"VOLTage:DC:RANGe", SCPIMM_sense_voltage_dc_range},
	{"VOLTage:DC:RANGe?", SCPIMM_sense_voltage_dc_rangeQ},
	{"VOLTage:AC:RANGe", SCPIMM_sense_voltage_ac_range},
	{"VOLTage:AC:RANGe?", SCPIMM_sense_voltage_ac_rangeQ},
	{"CURRent:DC:RANGe", SCPIMM_sense_current_dc_range},
	{"CURRent:DC:RANGe?", SCPIMM_sense_current_dc_rangeQ},
	{"CURRent:AC:RANGe", SCPIMM_sense_current_ac_range},
	{"CURRent:AC:RANGe?", SCPIMM_sense_current_ac_rangeQ},
	{"RESistance:RANGe", SCPIMM_sense_resistance_range},
	{"RESistance:RANGe?", SCPIMM_sense_resistance_rangeQ},
	{"FRESistance:RANGe", SCPIMM_sense_fresistance_range},
	{"FRESistance:RANGe?", SCPIMM_sense_fresistance_rangeQ},
	{"FREQuency:VOLTage:RANGe", SCPIMM_sense_frequency_voltage_range},
	{"FREQuency:VOLTage:RANGe?", SCPIMM_sense_frequency_voltage_rangeQ},
	{"PERiod:VOLTage:RANGe", SCPIMM_sense_period_voltage_range},
	{"PERiod:VOLTage:RANGe?", SCPIMM_sense_period_voltage_rangeQ},
	{"VOLTage:DC:RANGe:AUTO", SCPIMM_sense_voltage_dc_range_auto},
	{"VOLTage:DC:RANGe:AUTO?", SCPIMM_sense_voltage_dc_range_autoQ},
	{"VOLTage:AC:RANGe:AUTO", SCPIMM_sense_voltage_ac_range_auto},
	{"VOLTage:AC:RANGe:AUTO?", SCPIMM_sense_voltage_ac_range_autoQ},
	{"CURRent:DC:RANGe:AUTO", SCPIMM_sense_current_dc_range_auto},
	{"CURRent:DC:RANGe:AUTO?", SCPIMM_sense_current_dc_range_autoQ},
	{"CURRent:AC:RANGe:AUTO", SCPIMM_sense_current_ac_range_auto},
	{"CURRent:AC:RANGe:AUTO?", SCPIMM_sense_current_ac_range_autoQ},
	{"RESistance:RANGe:AUTO", SCPIMM_sense_resistance_range_auto},
	{"RESistance:RANGe:AUTO?", SCPIMM_sense_resistance_range_autoQ},
	{"FRESistance:RANGe:AUTO", SCPIMM_sense_fresistance_range_auto},
	{"FRESistance:RANGe:AUTO?", SCPIMM_sense_fresistance_range_autoQ},
	{"FREQuency:VOLTage:RANGe:AUTO", SCPIMM_sense_frequency_voltage_range_auto},
	{"FREQuency:VOLTage:RANGe:AUTO?", SCPIMM_sense_frequency_voltage_range_autoQ},
	{"PERiod:VOLTage:RANGe:AUTO", SCPIMM_sense_period_voltage_range_auto},
	{"PERiod:VOLTage:RANGe:AUTO?", SCPIMM_sense_period_voltage_range_autoQ},
	{"VOLTage:DC:RESolution", SCPIMM_sense_voltage_dc_resolution},
	{"VOLTage:DC:RESolution?", SCPIMM_sense_voltage_dc_resolutionQ},
	{"VOLTage:AC:RESolution", SCPIMM_sense_voltage_ac_resolution},
	{"VOLTage:AC:RESolution?", SCPIMM_sense_voltage_ac_resolutionQ},
	{"CURRent:DC:RESolution", SCPIMM_sense_current_dc_resolution},
	{"CURRent:DC:RESolution?", SCPIMM_sense_current_dc_resolutionQ},
	{"CURRent:AC:RESolution", SCPIMM_sense_current_ac_resolution},
	{"CURRent:AC:RESolution?", SCPIMM_sense_current_ac_resolutionQ},
	{"RESistance:RESolution", SCPIMM_sense_resistance_resolution},
	{"RESistance:RESolution?", SCPIMM_sense_resistance_resolutionQ},
	{"FRESistance:RESolution", SCPIMM_sense_fresistance_resolution},
	{"FRESistance:RESolution?", SCPIMM_sense_fresistance_resolutionQ},
	{"VOLTage:DC:NPLCycles", SCPIMM_sense_voltage_dc_nplcycles},
	{"VOLTage:DC:NPLCycles?", SCPIMM_sense_voltage_dc_nplcyclesQ},
	{"CURRent:DC:NPLCycles", SCPIMM_sense_current_dc_nplcycles},
	{"CURRent:DC:NPLCycles?", SCPIMM_sense_current_dc_nplcyclesQ},
	{"RESistance:NPLCycles", SCPIMM_sense_resistance_nplcycles},
	{"RESistance:NPLCycles?", SCPIMM_sense_resistance_nplcyclesQ},
	{"FRESistance:NPLCycles", SCPIMM_sense_fresistance_nplcycles},
	{"FRESistance:NPLCycles?", SCPIMM_sense_fresistance_nplcyclesQ},
	{"FREQuency:APERture", SCPIMM_sense_frequency_aperture},
	{"FREQuency:APERture?", SCPIMM_sense_frequency_apertureQ},
	{"PERiod:APERture", SCPIMM_sense_period_aperture},
	{"PERiod:APERture?", SCPIMM_sense_period_apertureQ},
	{"DETector:BANDwidth", SCPIMM_sense_detector_bandwidth},
	{"DETector:BANDwidth?", SCPIMM_sense_detector_bandwidthQ},
	{"ZERO:AUTO", SCPIMM_sense_zero_auto},
	{"ZERO:AUTO?", SCPIMM_sense_zero_autoQ},

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
	SCPIMM_MODE_DCV
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
    scpi_units_def,
    scpi_special_numbers_def,
	&scpimm_context
};

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

	ctx->dcv_range.type = SCPI_NUM_DEF;
	ctx->dcv_ratio_range.type = SCPI_NUM_DEF;
	ctx->acv_range.type = SCPI_NUM_DEF;
	ctx->dcc_range.type = SCPI_NUM_DEF;
	ctx->acc_range.type = SCPI_NUM_DEF;
	ctx->resistance_range.type = SCPI_NUM_DEF;
	ctx->fresistance_range.type = SCPI_NUM_DEF;
	ctx->frequency_range.type = SCPI_NUM_DEF;
	ctx->period_range.type = SCPI_NUM_DEF;

	ctx->dcv_resolution.type = SCPI_NUM_DEF;
	ctx->dcv_ratio_resolution.type = SCPI_NUM_DEF;
	ctx->acv_resolution.type = SCPI_NUM_DEF;
	ctx->dcc_resolution.type = SCPI_NUM_DEF;
	ctx->acc_resolution.type = SCPI_NUM_DEF;
	ctx->resistance_resolution.type = SCPI_NUM_DEF;
	ctx->fresistance_resolution.type = SCPI_NUM_DEF;
	ctx->frequency_resolution.type = SCPI_NUM_DEF;
	ctx->period_resolution.type = SCPI_NUM_DEF;

	SCPIMM_do_configure(context, SCPIMM_MODE_DCV, &ctx->dcv_range, &ctx->dcv_resolution);

	// TODO

    return SCPI_RES_OK;
}

static scpi_result_t system_versionQ(scpi_t* context) {
	SCPI_ResultString(context, SCPIMM_VERSION);
	return SCPI_RES_OK;
}

