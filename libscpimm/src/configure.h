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

#ifndef __CONFIGURE_H_SCPIMM_
#define	__CONFIGURE_H_SCPIMM_

#include <scpi/scpi.h>
#include <scpimm/scpimm.h>

scpi_result_t SCPIMM_do_configure(scpi_t* context, scpimm_mode_t mode, const scpi_number_t* range, const scpi_number_t* resolution);

scpi_result_t SCPIMM_configureQ(scpi_t* context);
scpi_result_t SCPIMM_configure_voltage_dc(scpi_t* context);
scpi_result_t SCPIMM_configure_voltage_dc_ratio(scpi_t* context);
scpi_result_t SCPIMM_configure_voltage_ac(scpi_t* context);
scpi_result_t SCPIMM_configure_voltage_ac_ratio(scpi_t* context);
scpi_result_t SCPIMM_configure_current_dc(scpi_t* context);
scpi_result_t SCPIMM_configure_current_ac(scpi_t* context);
scpi_result_t SCPIMM_configure_resistance(scpi_t* context);
scpi_result_t SCPIMM_configure_fresistance(scpi_t* context);

#endif	//	__CONFIGURE_H_SCPIMM_

