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
    {"CONFigure:VOLTage", SCPIMM_configure_voltage_dc},
    {"CONFigure:VOLTage:DC", SCPIMM_configure_voltage_dc},
    {"CONFigure:VOLTage:DC:RATio", SCPIMM_configure_voltage_dc_ratio},
    {"CONFigure:VOLTage:AC", SCPIMM_configure_voltage_ac},
#ifndef SCPIMM_NO_CURR_DC
    {"CONFigure:CURRent", SCPIMM_configure_current_dc},
    {"CONFigure:CURRent:DC", SCPIMM_configure_current_dc},
#endif
#ifndef SCPIMM_NO_CURR_AC
    {"CONFigure:CURRent:AC", SCPIMM_configure_current_ac},
#endif
    {"CONFigure:RESistance", SCPIMM_configure_resistance},
#ifndef SCPIMM_NO_FRESISTANCE
    {"CONFigure:FRESistance", SCPIMM_configure_fresistance},
#endif
#ifndef SCPIMM_NO_FREQUENCY
    {"CONFigure:FREQuency", SCPIMM_configure_frequency},
#endif
#ifndef SCPIMM_NO_PERIOD
    {"CONFigure:PERiod", SCPIMM_configure_period},
#endif
#ifndef SCPIMM_NO_CONTINUITY
    {"CONFigure:CONTinuity", SCPIMM_configure_continuity},
#endif
#ifndef SCPIMM_NO_DIODE
    {"CONFigure:DIODe", SCPIMM_configure_diode},
#endif

    {"DISPlay", SCPIMM_display},
    {"DISPlay?", SCPIMM_displayQ},
    {"DISPlay:TEXT", SCPIMM_display_text},
    {"DISPlay:TEXT?", SCPIMM_display_textQ},
    {"DISPlay:TEXT:CLEar", SCPIMM_display_text_clear},

    {"INPut:IMPedance:AUTO", SCPIMM_input_impedance_auto},
    {"INPut:IMPedance:AUTO?", SCPIMM_input_impedance_autoQ},

    {"MEASure:VOLTage?", SCPIMM_measure_voltage_dcQ},
    {"MEASure:VOLTage:DC?", SCPIMM_measure_voltage_dcQ},
    {"MEASure:VOLTage:DC:RATio?", SCPIMM_measure_voltage_dc_ratioQ},
    {"MEASure:VOLTage:AC?", SCPIMM_measure_voltage_acQ},
#ifndef SCPIMM_NO_CURR_DC
    {"MEASure:CURRent?", SCPIMM_measure_current_dcQ},
    {"MEASure:CURRent:DC?", SCPIMM_measure_current_dcQ},
#endif
#ifndef SCPIMM_NO_CURR_AC
    {"MEASure:CURRent:AC?", SCPIMM_measure_current_acQ},
#endif
    {"MEASure:RESistance?", SCPIMM_measure_resistanceQ},
#ifndef SCPIMM_NO_FRESISTANCE
    {"MEASure:FRESistance?", SCPIMM_measure_fresistanceQ},
#endif
#ifndef SCPIMM_NO_FREQUENCY
    {"MEASure:FREQuency?", SCPIMM_measure_frequencyQ},
#endif
#ifndef SCPIMM_NO_PERIOD
    {"MEASure:PERiod?", SCPIMM_measure_periodQ},
#endif
#ifndef SCPIMM_NO_CONTINUITY
    {"MEASure:CONTinuity?", SCPIMM_measure_continuityQ},
#endif
#ifndef SCPIMM_NO_DIODE
    {"MEASure:DIODe?", SCPIMM_measure_diodeQ},
#endif

	{"SAMPle:COUNt", SCPIMM_sample_count},
	{"SAMPle:COUNt?", SCPIMM_sample_countQ},

	{"SENSe:FUNCtion", SCPIMM_sense_function},
	{"SENSe:FUNCtion?", SCPIMM_sense_functionQ},
	{"SENSe:VOLTage:RANGe", SCPIMM_sense_voltage_dc_range},
	{"SENSe:VOLTage:RANGe?", SCPIMM_sense_voltage_dc_rangeQ},
	{"SENSe:VOLTage:DC:RANGe", SCPIMM_sense_voltage_dc_range},
	{"SENSe:VOLTage:DC:RANGe?", SCPIMM_sense_voltage_dc_rangeQ},
	{"SENSe:VOLTage:AC:RANGe", SCPIMM_sense_voltage_ac_range},
	{"SENSe:VOLTage:AC:RANGe?", SCPIMM_sense_voltage_ac_rangeQ},
#ifndef SCPIMM_NO_CURR_DC
	{"SENSe:CURRent:RANGe", SCPIMM_sense_current_dc_range},
	{"SENSe:CURRent:RANGe?", SCPIMM_sense_current_dc_rangeQ},
	{"SENSe:CURRent:DC:RANGe", SCPIMM_sense_current_dc_range},
	{"SENSe:CURRent:DC:RANGe?", SCPIMM_sense_current_dc_rangeQ},
#endif
#ifndef SCPIMM_NO_CURR_AC
	{"SENSe:CURRent:AC:RANGe", SCPIMM_sense_current_ac_range},
	{"SENSe:CURRent:AC:RANGe?", SCPIMM_sense_current_ac_rangeQ},
#endif
	{"SENSe:RESistance:RANGe", SCPIMM_sense_resistance_range},
	{"SENSe:RESistance:RANGe?", SCPIMM_sense_resistance_rangeQ},
#ifndef SCPIMM_NO_FRESISTANCE
	{"SENSe:FRESistance:RANGe", SCPIMM_sense_fresistance_range},
	{"SENSe:FRESistance:RANGe?", SCPIMM_sense_fresistance_rangeQ},
#endif
#ifndef SCPIMM_NO_FREQUENCY
	{"SENSe:FREQuency:VOLTage:RANGe", SCPIMM_sense_frequency_voltage_range},
	{"SENSe:FREQuency:VOLTage:RANGe?", SCPIMM_sense_frequency_voltage_rangeQ},
#endif
#ifndef SCPIMM_NO_PERIOD
	{"SENSe:PERiod:VOLTage:RANGe", SCPIMM_sense_period_voltage_range},
	{"SENSe:PERiod:VOLTage:RANGe?", SCPIMM_sense_period_voltage_rangeQ},
#endif
	{"SENSe:VOLTage:RANGe:AUTO", SCPIMM_sense_voltage_dc_range_auto},
	{"SENSe:VOLTage:RANGe:AUTO?", SCPIMM_sense_voltage_dc_range_autoQ},
	{"SENSe:VOLTage:DC:RANGe:AUTO", SCPIMM_sense_voltage_dc_range_auto},
	{"SENSe:VOLTage:DC:RANGe:AUTO?", SCPIMM_sense_voltage_dc_range_autoQ},
	{"SENSe:VOLTage:AC:RANGe:AUTO", SCPIMM_sense_voltage_ac_range_auto},
	{"SENSe:VOLTage:AC:RANGe:AUTO?", SCPIMM_sense_voltage_ac_range_autoQ},
#ifndef SCPIMM_NO_CURR_DC
	{"SENSe:CURRent:RANGe:AUTO", SCPIMM_sense_current_dc_range_auto},
	{"SENSe:CURRent:RANGe:AUTO?", SCPIMM_sense_current_dc_range_autoQ},
	{"SENSe:CURRent:DC:RANGe:AUTO", SCPIMM_sense_current_dc_range_auto},
	{"SENSe:CURRent:DC:RANGe:AUTO?", SCPIMM_sense_current_dc_range_autoQ},
#endif
#ifndef SCPIMM_NO_CURR_AC
	{"SENSe:CURRent:AC:RANGe:AUTO", SCPIMM_sense_current_ac_range_auto},
	{"SENSe:CURRent:AC:RANGe:AUTO?", SCPIMM_sense_current_ac_range_autoQ},
#endif
	{"SENSe:RESistance:RANGe:AUTO", SCPIMM_sense_resistance_range_auto},
	{"SENSe:RESistance:RANGe:AUTO?", SCPIMM_sense_resistance_range_autoQ},
#ifndef SCPIMM_NO_FRESISTANCE
	{"SENSe:FRESistance:RANGe:AUTO", SCPIMM_sense_fresistance_range_auto},
	{"SENSe:FRESistance:RANGe:AUTO?", SCPIMM_sense_fresistance_range_autoQ},
#endif
#ifndef SCPIMM_NO_FREQUENCY
	{"SENSe:FREQuency:VOLTage:RANGe:AUTO", SCPIMM_sense_frequency_voltage_range_auto},
	{"SENSe:FREQuency:VOLTage:RANGe:AUTO?", SCPIMM_sense_frequency_voltage_range_autoQ},
#endif
#ifndef SCPIMM_NO_PERIOD
	{"SENSe:PERiod:VOLTage:RANGe:AUTO", SCPIMM_sense_period_voltage_range_auto},
	{"SENSe:PERiod:VOLTage:RANGe:AUTO?", SCPIMM_sense_period_voltage_range_autoQ},
#endif
	{"SENSe:VOLTage:RESolution", SCPIMM_sense_voltage_dc_resolution},
	{"SENSe:VOLTage:RESolution?", SCPIMM_sense_voltage_dc_resolutionQ},
	{"SENSe:VOLTage:DC:RESolution", SCPIMM_sense_voltage_dc_resolution},
	{"SENSe:VOLTage:DC:RESolution?", SCPIMM_sense_voltage_dc_resolutionQ},
	{"SENSe:VOLTage:AC:RESolution", SCPIMM_sense_voltage_ac_resolution},
	{"SENSe:VOLTage:AC:RESolution?", SCPIMM_sense_voltage_ac_resolutionQ},
#ifndef SCPIMM_NO_CURR_DC
	{"SENSe:CURRent:RESolution", SCPIMM_sense_current_dc_resolution},
	{"SENSe:CURRent:RESolution?", SCPIMM_sense_current_dc_resolutionQ},
	{"SENSe:CURRent:DC:RESolution", SCPIMM_sense_current_dc_resolution},
	{"SENSe:CURRent:DC:RESolution?", SCPIMM_sense_current_dc_resolutionQ},
#endif
#ifndef SCPIMM_NO_CURR_AC
	{"SENSe:CURRent:AC:RESolution", SCPIMM_sense_current_ac_resolution},
	{"SENSe:CURRent:AC:RESolution?", SCPIMM_sense_current_ac_resolutionQ},
#endif
	{"SENSe:RESistance:RESolution", SCPIMM_sense_resistance_resolution},
	{"SENSe:RESistance:RESolution?", SCPIMM_sense_resistance_resolutionQ},
#ifndef SCPIMM_NO_FRESISTANCE
	{"SENSe:FRESistance:RESolution", SCPIMM_sense_fresistance_resolution},
	{"SENSe:FRESistance:RESolution?", SCPIMM_sense_fresistance_resolutionQ},
#endif
	{"SENSe:VOLTage:NPLCycles", SCPIMM_sense_voltage_dc_nplcycles},
	{"SENSe:VOLTage:NPLCycles?", SCPIMM_sense_voltage_dc_nplcyclesQ},
	{"SENSe:VOLTage:DC:NPLCycles", SCPIMM_sense_voltage_dc_nplcycles},
	{"SENSe:VOLTage:DC:NPLCycles?", SCPIMM_sense_voltage_dc_nplcyclesQ},
#ifndef SCPIMM_NO_CURR_DC
	{"SENSe:CURRent:NPLCycles", SCPIMM_sense_current_dc_nplcycles},
	{"SENSe:CURRent:NPLCycles?", SCPIMM_sense_current_dc_nplcyclesQ},
	{"SENSe:CURRent:DC:NPLCycles", SCPIMM_sense_current_dc_nplcycles},
	{"SENSe:CURRent:DC:NPLCycles?", SCPIMM_sense_current_dc_nplcyclesQ},
#endif
	{"SENSe:RESistance:NPLCycles", SCPIMM_sense_resistance_nplcycles},
	{"SENSe:RESistance:NPLCycles?", SCPIMM_sense_resistance_nplcyclesQ},
#ifndef SCPIMM_NO_FRESISTANCE
	{"SENSe:FRESistance:NPLCycles", SCPIMM_sense_fresistance_nplcycles},
	{"SENSe:FRESistance:NPLCycles?", SCPIMM_sense_fresistance_nplcyclesQ},
#endif
#ifndef SCPIMM_NO_FREQUENCY
	{"SENSe:FREQuency:APERture", SCPIMM_sense_frequency_aperture},
	{"SENSe:FREQuency:APERture?", SCPIMM_sense_frequency_apertureQ},
#endif
#ifndef SCPIMM_NO_PERIOD
	{"SENSe:PERiod:APERture", SCPIMM_sense_period_aperture},
	{"SENSe:PERiod:APERture?", SCPIMM_sense_period_apertureQ},
#endif
	{"SENSe:DETector:BANDwidth", SCPIMM_sense_detector_bandwidth},
	{"SENSe:DETector:BANDwidth?", SCPIMM_sense_detector_bandwidthQ},
	{"SENSe:ZERO:AUTO", SCPIMM_sense_zero_auto},
	{"SENSe:ZERO:AUTO?", SCPIMM_sense_zero_autoQ},

	{"FUNCtion", SCPIMM_sense_function},
	{"FUNCtion?", SCPIMM_sense_functionQ},
	{"VOLTage:RANGe", SCPIMM_sense_voltage_dc_range},
	{"VOLTage:RANGe?", SCPIMM_sense_voltage_dc_rangeQ},
	{"VOLTage:DC:RANGe", SCPIMM_sense_voltage_dc_range},
	{"VOLTage:DC:RANGe?", SCPIMM_sense_voltage_dc_rangeQ},
	{"VOLTage:AC:RANGe", SCPIMM_sense_voltage_ac_range},
	{"VOLTage:AC:RANGe?", SCPIMM_sense_voltage_ac_rangeQ},
#ifndef SCPIMM_NO_CURR_DC
	{"CURRent:RANGe", SCPIMM_sense_current_dc_range},
	{"CURRent:RANGe?", SCPIMM_sense_current_dc_rangeQ},
	{"CURRent:DC:RANGe", SCPIMM_sense_current_dc_range},
	{"CURRent:DC:RANGe?", SCPIMM_sense_current_dc_rangeQ},
#endif
#ifndef SCPIMM_NO_CURR_AC
	{"CURRent:AC:RANGe", SCPIMM_sense_current_ac_range},
	{"CURRent:AC:RANGe?", SCPIMM_sense_current_ac_rangeQ},
#endif
	{"RESistance:RANGe", SCPIMM_sense_resistance_range},
	{"RESistance:RANGe?", SCPIMM_sense_resistance_rangeQ},
#ifndef SCPIMM_NO_FRESISTANCE
	{"FRESistance:RANGe", SCPIMM_sense_fresistance_range},
	{"FRESistance:RANGe?", SCPIMM_sense_fresistance_rangeQ},
#endif
#ifndef SCPIMM_NO_FREQUENCY
	{"FREQuency:VOLTage:RANGe", SCPIMM_sense_frequency_voltage_range},
	{"FREQuency:VOLTage:RANGe?", SCPIMM_sense_frequency_voltage_rangeQ},
#endif
#ifndef SCPIMM_NO_PERIOD
	{"PERiod:VOLTage:RANGe", SCPIMM_sense_period_voltage_range},
	{"PERiod:VOLTage:RANGe?", SCPIMM_sense_period_voltage_rangeQ},
#endif
	{"VOLTage:RANGe:AUTO", SCPIMM_sense_voltage_dc_range_auto},
	{"VOLTage:RANGe:AUTO?", SCPIMM_sense_voltage_dc_range_autoQ},
	{"VOLTage:DC:RANGe:AUTO", SCPIMM_sense_voltage_dc_range_auto},
	{"VOLTage:DC:RANGe:AUTO?", SCPIMM_sense_voltage_dc_range_autoQ},
	{"VOLTage:AC:RANGe:AUTO", SCPIMM_sense_voltage_ac_range_auto},
	{"VOLTage:AC:RANGe:AUTO?", SCPIMM_sense_voltage_ac_range_autoQ},
#ifndef SCPIMM_NO_CURR_DC
	{"CURRent:RANGe:AUTO", SCPIMM_sense_current_dc_range_auto},
	{"CURRent:RANGe:AUTO?", SCPIMM_sense_current_dc_range_autoQ},
	{"CURRent:DC:RANGe:AUTO", SCPIMM_sense_current_dc_range_auto},
	{"CURRent:DC:RANGe:AUTO?", SCPIMM_sense_current_dc_range_autoQ},
#endif
#ifndef SCPIMM_NO_CURR_AC
	{"CURRent:AC:RANGe:AUTO", SCPIMM_sense_current_ac_range_auto},
	{"CURRent:AC:RANGe:AUTO?", SCPIMM_sense_current_ac_range_autoQ},
#endif
	{"RESistance:RANGe:AUTO", SCPIMM_sense_resistance_range_auto},
	{"RESistance:RANGe:AUTO?", SCPIMM_sense_resistance_range_autoQ},
#ifndef SCPIMM_NO_FRESISTANCE
	{"FRESistance:RANGe:AUTO", SCPIMM_sense_fresistance_range_auto},
	{"FRESistance:RANGe:AUTO?", SCPIMM_sense_fresistance_range_autoQ},
#endif
#ifndef SCPIMM_NO_FREQUENCY
	{"FREQuency:VOLTage:RANGe:AUTO", SCPIMM_sense_frequency_voltage_range_auto},
	{"FREQuency:VOLTage:RANGe:AUTO?", SCPIMM_sense_frequency_voltage_range_autoQ},
#endif
#ifndef SCPIMM_NO_PERIOD
	{"PERiod:VOLTage:RANGe:AUTO", SCPIMM_sense_period_voltage_range_auto},
	{"PERiod:VOLTage:RANGe:AUTO?", SCPIMM_sense_period_voltage_range_autoQ},
#endif
	{"VOLTage:RESolution", SCPIMM_sense_voltage_dc_resolution},
	{"VOLTage:RESolution?", SCPIMM_sense_voltage_dc_resolutionQ},
	{"VOLTage:DC:RESolution", SCPIMM_sense_voltage_dc_resolution},
	{"VOLTage:DC:RESolution?", SCPIMM_sense_voltage_dc_resolutionQ},
	{"VOLTage:AC:RESolution", SCPIMM_sense_voltage_ac_resolution},
	{"VOLTage:AC:RESolution?", SCPIMM_sense_voltage_ac_resolutionQ},
#ifndef SCPIMM_NO_CURR_DC
	{"CURRent:RESolution", SCPIMM_sense_current_dc_resolution},
	{"CURRent:RESolution?", SCPIMM_sense_current_dc_resolutionQ},
	{"CURRent:DC:RESolution", SCPIMM_sense_current_dc_resolution},
	{"CURRent:DC:RESolution?", SCPIMM_sense_current_dc_resolutionQ},
#endif
#ifndef SCPIMM_NO_CURR_AC
	{"CURRent:AC:RESolution", SCPIMM_sense_current_ac_resolution},
	{"CURRent:AC:RESolution?", SCPIMM_sense_current_ac_resolutionQ},
#endif
	{"RESistance:RESolution", SCPIMM_sense_resistance_resolution},
	{"RESistance:RESolution?", SCPIMM_sense_resistance_resolutionQ},
#ifndef SCPIMM_NO_FRESISTANCE
	{"FRESistance:RESolution", SCPIMM_sense_fresistance_resolution},
	{"FRESistance:RESolution?", SCPIMM_sense_fresistance_resolutionQ},
#endif
	{"VOLTage:NPLCycles", SCPIMM_sense_voltage_dc_nplcycles},
	{"VOLTage:NPLCycles?", SCPIMM_sense_voltage_dc_nplcyclesQ},
	{"VOLTage:DC:NPLCycles", SCPIMM_sense_voltage_dc_nplcycles},
	{"VOLTage:DC:NPLCycles?", SCPIMM_sense_voltage_dc_nplcyclesQ},
#ifndef SCPIMM_NO_CURR_DC
	{"CURRent:NPLCycles", SCPIMM_sense_current_dc_nplcycles},
	{"CURRent:NPLCycles?", SCPIMM_sense_current_dc_nplcyclesQ},
	{"CURRent:DC:NPLCycles", SCPIMM_sense_current_dc_nplcycles},
	{"CURRent:DC:NPLCycles?", SCPIMM_sense_current_dc_nplcyclesQ},
#endif
	{"RESistance:NPLCycles", SCPIMM_sense_resistance_nplcycles},
	{"RESistance:NPLCycles?", SCPIMM_sense_resistance_nplcyclesQ},
#ifndef SCPIMM_NO_FRESISTANCE
	{"FRESistance:NPLCycles", SCPIMM_sense_fresistance_nplcycles},
	{"FRESistance:NPLCycles?", SCPIMM_sense_fresistance_nplcyclesQ},
#endif
#ifndef SCPIMM_NO_FREQUENCY
	{"FREQuency:APERture", SCPIMM_sense_frequency_aperture},
	{"FREQuency:APERture?", SCPIMM_sense_frequency_apertureQ},
#endif
#ifndef SCPIMM_NO_PERIOD
	{"PERiod:APERture", SCPIMM_sense_period_aperture},
	{"PERiod:APERture?", SCPIMM_sense_period_apertureQ},
#endif
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
	NULL
};

/*static scpi_reg_val_t scpi_regs[SCPI_REG_COUNT];*/

/******************************************************************************
  Global variables
******************************************************************************/

static volatile scpimm_context_t scpimm_context = {
	.interface = NULL,
	.beeper_state = TRUE
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
	(void*) &scpimm_context
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
	SCPIMM_set_remote(context, FALSE, FALSE);
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

static scpi_result_t system_versionQ(scpi_t* context) {
	SCPI_ResultString(context, SCPIMM_VERSION);
	return SCPI_RES_OK;
}

