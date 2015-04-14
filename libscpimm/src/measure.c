/*
    Copyright (c) 2015 Andrey Nakin
    All Rights Reserved

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

#include "measure.h"
#include "configure.h"
#include "utils.h"
#include "dmm.h"

static scpi_result_t do_measure(scpi_t* const context, scpi_command_callback_t const mode_switcher) {
	scpi_result_t result = mode_switcher(context);
	if (SCPI_RES_OK == result) {
		result = SCPIMM_readQ(context);
	}
    return result;
}

scpi_result_t SCPIMM_measure_voltage_dcQ(scpi_t* const context) {
	return do_measure(context, SCPIMM_configure_voltage_dc);
}

scpi_result_t SCPIMM_measure_voltage_dc_ratioQ(scpi_t* context) {
	return do_measure(context, SCPIMM_configure_voltage_dc_ratio);
}

scpi_result_t SCPIMM_measure_voltage_acQ(scpi_t* context) {
	return do_measure(context, SCPIMM_configure_voltage_ac);
}

scpi_result_t SCPIMM_measure_voltage_ac_ratioQ(scpi_t* context) {
	return do_measure(context, SCPIMM_configure_voltage_ac_ratio);
}

scpi_result_t SCPIMM_measure_current_dcQ(scpi_t* context) {
	return do_measure(context, SCPIMM_configure_current_dc);
}

scpi_result_t SCPIMM_measure_current_acQ(scpi_t* context) {
	return do_measure(context, SCPIMM_configure_current_ac);
}

scpi_result_t SCPIMM_measure_resistanceQ(scpi_t* context) {
	return do_measure(context, SCPIMM_configure_resistance);
}

scpi_result_t SCPIMM_measure_fresistanceQ(scpi_t* context) {
	return do_measure(context, SCPIMM_configure_fresistance);
}
