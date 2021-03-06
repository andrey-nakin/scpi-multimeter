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

#ifndef __DMM_H_SCPIMM_
#define	__DMM_H_SCPIMM_

#include <scpi/scpi.h>
#include "scpimm_internal.h"

void SCPIMM_read_value(const scpi_number_t* value);

scpi_result_t SCPIMM_initiate(scpi_t* context);
scpi_result_t SCPIMM_readQ(scpi_t* context);
scpi_result_t SCPIMM_fetchQ(scpi_t* context);
scpi_result_t SCPIMM_trg(scpi_t* context);
scpi_result_t SCPIMM_data_pointsQ(scpi_t* context);

scpi_result_t SCPIMM_measure_preset(scpi_t* context);
void SCPIMM_stop_mesurement(volatile scpimm_context_t* ctx);
void SCPIMM_clear_return_buffer(scpi_t* context);
scpimm_error_t SCPIMM_wait_for_idle(volatile scpimm_context_t* const ctx);
scpimm_error_t SCPIMM_set_mode(scpi_t* context, scpimm_mode_t mode, const scpi_number_t* range, scpi_bool_t auto_detect_auto_range, const scpi_bool_t* auto_range, const scpi_number_t* resolution);
const char* SCPIMM_mode_name(scpimm_mode_t mode);

#endif	//	__DMM_H_SCPIMM_

