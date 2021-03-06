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

#ifndef	_DEFAULT_MULTIMETER_H
#define	_DEFAULT_MULTIMETER_H

/**
 * Dummy multimeter implementation
 */

#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "../src/scpimm_internal.h"

typedef enum {
	DM_MEASUREMENT_TYPE_ASYNC, DM_MEASUREMENT_TYPE_SYNC
} dm_measurement_type_t;

typedef struct {
	size_t range_index;
	size_t resolution_index;
	size_t nplc_index;
	scpi_bool_t auto_range;
} dm_mode_state_t;

typedef struct {
	scpimm_mode_t mode;
	scpi_bool_t mode_initialized;
	unsigned interrrupt_disable_counter;
	unsigned measurement_failure_counter;
	scpimm_terminal_state_t terminal_state;
	scpi_bool_t input_impedance_auto_state;
	scpi_bool_t zero_auto, zero_auto_once;
	scpi_bool_t remote, lock;

	struct {
		dm_mode_state_t dcv, dcv_ratio, acv, acv_ratio, dcc, acc, resistance, fresistance;
	} mode_states;

} dm_multimeter_state_t;

typedef struct {

	struct {
		scpimm_mode_t mode;
		scpimm_mode_params_t params;
		scpi_bool_t params_is_null;
	} set_mode;

	struct {
		scpimm_bool_param_t param;
		scpi_bool_t value_is_null;
	} get_global_bool_param;

	struct {
		scpimm_bool_param_t param;
		scpi_bool_t value;
	} set_global_bool_param;

	struct {
		scpimm_mode_t mode;
		scpimm_numeric_param_t param;
		scpi_bool_t values_is_null;
	} get_numeric_param_values;

	struct {
		scpimm_mode_t mode;
		scpimm_numeric_param_t param;
		scpi_bool_t value_is_null;
	} get_numeric_param;

	struct {
		scpimm_mode_t mode;
		scpimm_numeric_param_t param;
		size_t value_index;
	} set_numeric_param;

	struct {
		scpimm_mode_t mode;
		scpimm_bool_param_t param;
		scpi_bool_t value_is_null;
	} get_bool_param;

	struct {
		scpimm_mode_t mode;
		scpimm_bool_param_t param;
		scpi_bool_t value;
	} set_bool_param;

	struct {
		scpimm_mode_t mode;
		size_t range_index;
		const double* resolutions;
		scpi_bool_t resolutions_is_null;
	} get_allowed_resolutions;

	struct {
		const char* txt;
	} display_text;

} dm_args_t;

typedef struct {
	unsigned set_mode, get_mode, get_allowed_resolutions,
		start_measure, set_interrupt_status, beep, display_text, setup, reset,
		get_global_bool_param, set_global_bool_param, get_bool_param, set_bool_param,
		get_numeric_param_values, get_numeric_param, set_numeric_param,
		test;
} dm_counters_t;

typedef struct {
	scpimm_error_t get_input_terminal,
	get_global_bool_param, set_global_bool_param,
	get_bool_param, set_bool_param,
	get_numeric_param_values, get_numeric_param, set_numeric_param,
	test;
} dm_returns_t;

typedef double (*dm_measurement_func_t)(uint32_t time);

typedef struct {
	dm_measurement_type_t measurement_type;
	dm_measurement_func_t measurement_func;
	uint32_t measurement_duration;
} dm_multimeter_config_t;

extern dm_multimeter_state_t dm_multimeter_state;
extern char dm_display[SCPIMM_DISPLAY_LEN + 1];

extern dm_args_t dm_args, dm_prev_args;

extern scpimm_interface_t dm_interface;
extern dm_counters_t dm_counters;
extern dm_returns_t dm_returns;
extern dm_multimeter_config_t dm_multimeter_config;

const char* dm_read_entire_output_buffer();
void dm_reset_counters();
void dm_reset_args();

double dm_measurement_func_const(uint32_t time);

#endif	//	_DEFAULT_MULTIMETER_H
