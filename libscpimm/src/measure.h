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

#ifndef __MEASURE_H_SCPIMM_
#define	__MEASURE_H_SCPIMM_

#include <scpi/scpi.h>

scpi_result_t SCPIMM_measure_voltage_dcQ(scpi_t* context);
scpi_result_t SCPIMM_measure_voltage_dc_ratioQ(scpi_t* context);
scpi_result_t SCPIMM_measure_voltage_acQ(scpi_t* context);
scpi_result_t SCPIMM_measure_voltage_ac_ratioQ(scpi_t* context);
scpi_result_t SCPIMM_measure_current_dcQ(scpi_t* context);
scpi_result_t SCPIMM_measure_current_acQ(scpi_t* context);
scpi_result_t SCPIMM_measure_resistanceQ(scpi_t* context);
scpi_result_t SCPIMM_measure_fresistanceQ(scpi_t* context);

#endif	//	__MEASURE_H_SCPIMM_

