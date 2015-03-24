#ifndef __SENSE_H_SCPIMM_
#define	__SENSE_H_SCPIMM_

#include <scpi/scpi.h>

scpi_result_t SCPIMM_sense_function(scpi_t* context);
scpi_result_t SCPIMM_sense_functionQ(scpi_t* context);
scpi_result_t SCPIMM_sense_voltage_dc_range(scpi_t* context);
scpi_result_t SCPIMM_sense_voltage_dc_rangeQ(scpi_t* context);
scpi_result_t SCPIMM_sense_voltage_ac_range(scpi_t* context);
scpi_result_t SCPIMM_sense_voltage_ac_rangeQ(scpi_t* context);
scpi_result_t SCPIMM_sense_current_dc_range(scpi_t* context);
scpi_result_t SCPIMM_sense_current_dc_rangeQ(scpi_t* context);
scpi_result_t SCPIMM_sense_current_ac_range(scpi_t* context);
scpi_result_t SCPIMM_sense_current_ac_rangeQ(scpi_t* context);
scpi_result_t SCPIMM_sense_resistance_range(scpi_t* context);
scpi_result_t SCPIMM_sense_resistance_rangeQ(scpi_t* context);
scpi_result_t SCPIMM_sense_fresistance_range(scpi_t* context);
scpi_result_t SCPIMM_sense_fresistance_rangeQ(scpi_t* context);
scpi_result_t SCPIMM_sense_voltage_dc_range_auto(scpi_t* context);
scpi_result_t SCPIMM_sense_voltage_dc_range_autoQ(scpi_t* context);
scpi_result_t SCPIMM_sense_voltage_ac_range_auto(scpi_t* context);
scpi_result_t SCPIMM_sense_voltage_ac_range_autoQ(scpi_t* context);
scpi_result_t SCPIMM_sense_current_dc_range_auto(scpi_t* context);
scpi_result_t SCPIMM_sense_current_dc_range_autoQ(scpi_t* context);
scpi_result_t SCPIMM_sense_current_ac_range_auto(scpi_t* context);
scpi_result_t SCPIMM_sense_current_ac_range_autoQ(scpi_t* context);
scpi_result_t SCPIMM_sense_resistance_range_auto(scpi_t* context);
scpi_result_t SCPIMM_sense_resistance_range_autoQ(scpi_t* context);
scpi_result_t SCPIMM_sense_fresistance_range_auto(scpi_t* context);
scpi_result_t SCPIMM_sense_fresistance_range_autoQ(scpi_t* context);

scpi_result_t SCPIMM_sense_voltage_dc_resolution(scpi_t* context);
scpi_result_t SCPIMM_sense_voltage_dc_resolutionQ(scpi_t* context);
scpi_result_t SCPIMM_sense_voltage_ac_resolution(scpi_t* context);
scpi_result_t SCPIMM_sense_voltage_ac_resolutionQ(scpi_t* context);
scpi_result_t SCPIMM_sense_current_dc_resolution(scpi_t* context);
scpi_result_t SCPIMM_sense_current_dc_resolutionQ(scpi_t* context);
scpi_result_t SCPIMM_sense_current_ac_resolution(scpi_t* context);
scpi_result_t SCPIMM_sense_current_ac_resolutionQ(scpi_t* context);
scpi_result_t SCPIMM_sense_resistance_resolution(scpi_t* context);
scpi_result_t SCPIMM_sense_resistance_resolutionQ(scpi_t* context);
scpi_result_t SCPIMM_sense_fresistance_resolution(scpi_t* context);
scpi_result_t SCPIMM_sense_fresistance_resolutionQ(scpi_t* context);

scpi_result_t SCPIMM_sense_voltage_dc_nplcycles(scpi_t* context);
scpi_result_t SCPIMM_sense_voltage_dc_nplcyclesQ(scpi_t* context);
scpi_result_t SCPIMM_sense_current_dc_nplcycles(scpi_t* context);
scpi_result_t SCPIMM_sense_current_dc_nplcyclesQ(scpi_t* context);
scpi_result_t SCPIMM_sense_resistance_nplcycles(scpi_t* context);
scpi_result_t SCPIMM_sense_resistance_nplcyclesQ(scpi_t* context);
scpi_result_t SCPIMM_sense_fresistance_nplcycles(scpi_t* context);
scpi_result_t SCPIMM_sense_fresistance_nplcyclesQ(scpi_t* context);

scpi_result_t SCPIMM_sense_zero_auto(scpi_t* context);
scpi_result_t SCPIMM_sense_zero_autoQ(scpi_t* context);

#endif	//	__SENSE_H_SCPIMM_

