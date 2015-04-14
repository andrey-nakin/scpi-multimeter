/*
	This file is part of scpi-multimeter library.

	v7-28-arduino is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	v7-28-arduino is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with v7-28-arduino.  If not, see <http://www.gnu.org/licenses/>.
 */

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

