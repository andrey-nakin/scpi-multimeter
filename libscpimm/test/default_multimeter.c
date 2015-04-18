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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "default_multimeter.h"

#define DEFAULT_MEAS_DURATION 500

static int16_t dm_setup();
static int16_t dm_reset();
static int16_t dm_set_mode(scpimm_mode_t mode, const scpimm_mode_params_t* const params);
static int16_t dm_get_mode(scpimm_mode_t* mode);
static int16_t dm_get_allowed_resolutions(scpimm_mode_t mode, size_t range_index, const double** resolutions);
static int16_t dm_start_measure();
static size_t dm_send(const uint8_t* buf, size_t len);
static int16_t dm_get_milliseconds(uint32_t* tm);
static int16_t dm_set_interrupt_status(scpi_bool_t disabled);
static int16_t dm_get_global_bool_param(scpimm_bool_param_t param, scpi_bool_t* value);
static int16_t dm_set_global_bool_param(scpimm_bool_param_t param, scpi_bool_t value);
static int16_t dm_get_bool_param(scpimm_mode_t mode, scpimm_bool_param_t param, scpi_bool_t* value);
static int16_t dm_set_bool_param(scpimm_mode_t mode, scpimm_bool_param_t param, scpi_bool_t value);
static int16_t dm_get_numeric_param_values(scpimm_mode_t mode, scpimm_numeric_param_t param, const double** values);
static int16_t dm_get_numeric_param(scpimm_mode_t mode, scpimm_numeric_param_t param, size_t* value_index);
static int16_t dm_set_numeric_param(scpimm_mode_t mode, scpimm_numeric_param_t param, size_t value_index);
static int16_t dm_beep();
static int16_t dm_get_input_terminal(scpimm_terminal_state_t* term);
static int16_t dm_display_text(const char* txt);
static int16_t dm_test();

/***************************************************************
 * Global variables
 **************************************************************/

dm_multimeter_state_t dm_multimeter_state;
char dm_display[SCPIMM_DISPLAY_LEN + 1];

dm_args_t dm_args, dm_prev_args;

scpimm_interface_t dm_interface = {
		.setup = dm_setup,
		.reset = dm_reset,
		.set_mode = dm_set_mode,
		.get_mode = dm_get_mode,
		.get_allowed_resolutions = dm_get_allowed_resolutions,
		.start_measure = dm_start_measure,
		.send = dm_send,
		.get_milliseconds = dm_get_milliseconds,
		.set_interrupt_status = dm_set_interrupt_status,
		.get_global_bool_param = dm_get_global_bool_param,
		.set_global_bool_param = dm_set_global_bool_param,
		.get_bool_param = dm_get_bool_param,
		.set_bool_param = dm_set_bool_param,
		.get_numeric_param_values = dm_get_numeric_param_values,
		.get_numeric_param = dm_get_numeric_param,
		.set_numeric_param = dm_set_numeric_param,
		.beep = dm_beep,
		.get_input_terminal = dm_get_input_terminal,
		.display_text = dm_display_text,
		.test = dm_test
};
dm_counters_t dm_counters;
dm_returns_t dm_returns;
dm_multimeter_config_t dm_multimeter_config = {
	.measurement_type = DM_MEASUREMENT_TYPE_ASYNC,
	.measurement_func = dm_measurement_func_const
};

static char out_buffer[1024];
static size_t out_buffer_counter = 0;
static char out_buffer_read[sizeof(out_buffer) / sizeof(out_buffer[0])];

static const double RANGES[] =   {0.1, 1.0, 10.0, 100.0, 1000.0, -1.0};
static const double OVERRUNS[] = {1.2, 1.2, 1.2,  1.2,   1.2,    -1.0};
static const double RESOLUTIONS[][5] = {
		{0.1e-6, 0.1e-5, 0.1e-4, 0.1e-3, -1.0},
		{1.0e-6, 1.0e-5, 1.0e-4, 1.0e-3, -1.0},
		{10e-6, 10e-5, 10e-4, 10e-3, -1.0},
		{100e-6, 100e-5, 100e-4, 100e-3, -1.0},
		{1000e-6, 1000e-5, 1000e-4, 1000e-3, -1.0}
};
static const double NPLCS[] = {0.2, 1.0, 10.0, 100.0, -1.0};

static pthread_t measure_thread, trigger_thread;
static scpi_bool_t measure_thread_created = FALSE;
static scpi_bool_t trigger_thread_created = FALSE;
static sem_t measure_sem;

/***************************************************************
 * Private functions
 **************************************************************/

static int16_t sleep_milliseconds(const uint32_t ms) {
	struct timespec delay;

	if (ms > 999) {
		delay.tv_sec = ms / 1000;
		delay.tv_nsec = ms % 1000 * 1000000;
	} else {
		delay.tv_sec = 0;
		delay.tv_nsec = ms * 1000000;
	}
	nanosleep(&delay, NULL);
	return SCPIMM_ERROR_OK;
}

static dm_mode_state_t* get_mode_state(const scpimm_mode_t mode) {
	switch (mode) {
	case SCPIMM_MODE_DCV:
		return &dm_multimeter_state.mode_states.dcv;
	case SCPIMM_MODE_DCV_RATIO:
		return &dm_multimeter_state.mode_states.dcv_ratio;
	case SCPIMM_MODE_ACV:
		return &dm_multimeter_state.mode_states.acv;
	case SCPIMM_MODE_ACV_RATIO:
		return &dm_multimeter_state.mode_states.acv_ratio;
	case SCPIMM_MODE_DCC:
		return &dm_multimeter_state.mode_states.dcc;
	case SCPIMM_MODE_ACC:
		return &dm_multimeter_state.mode_states.acc;
	case SCPIMM_MODE_RESISTANCE_2W:
		return &dm_multimeter_state.mode_states.resistance;
	case SCPIMM_MODE_RESISTANCE_4W:
		return &dm_multimeter_state.mode_states.fresistance;
	}
	return NULL;
}

static uint32_t get_milliseconds() {
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	return tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
}

static void do_measurement() {
	scpi_number_t number = {0.0, SCPI_UNIT_NONE, SCPI_NUM_NUMBER};
	number.value = dm_multimeter_config.measurement_func(get_milliseconds());
	SCPIMM_read_value(&number);
}

static void* measure_thread_routine(void* args) {
	(void) args;	//	suppress warning

	while (TRUE) {
		sem_wait(&measure_sem);

		sleep_milliseconds(dm_multimeter_config.measurement_duration);	//	emulate real measurement delay
		do_measurement();
	}

	return NULL;
}

static void* trigger_thread_routine(void* args) {
	(void) args;	//	suppress warning

	while (TRUE) {
		sleep_milliseconds(100);
		SCPIMM_external_trigger();
	}

	return NULL;
}

static int16_t validate_mode(const scpimm_mode_t mode) {
	switch (mode) {
	case SCPIMM_MODE_DCV:
	case SCPIMM_MODE_DCV_RATIO:
	case SCPIMM_MODE_ACV:
	case SCPIMM_MODE_ACV_RATIO:
	case SCPIMM_MODE_DCC:
	case SCPIMM_MODE_ACC:
	case SCPIMM_MODE_RESISTANCE_2W:
	case SCPIMM_MODE_RESISTANCE_4W:
		return SCPIMM_ERROR_OK;
	}

	return SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE;
}

static size_t max_resolution_index(size_t range_index) {
	const double* resolutions = RESOLUTIONS[range_index];
	size_t resolution_index = 0;

	for (; resolutions[resolution_index] >= 0.0; resolution_index++) {};
	return --resolution_index;
}

static int16_t set_mode_impl(const scpimm_mode_t mode, const scpimm_mode_params_t* const params) {
	int16_t err;

	if (SCPIMM_ERROR_OK != (err = validate_mode(mode))) {
		return err;
	}

	if (params) {
		dm_mode_state_t* const mode_state = get_mode_state(mode);

		if (!mode_state) {
			return SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE;
		}

		if (sizeof(RANGES) / sizeof(RANGES[0]) - 1 <= params->range_index) {
			return SCPIMM_ERROR_DATA_OUT_OF_RANGE;
		}
		if (max_resolution_index(params->range_index) < params->resolution_index) {
			return SCPIMM_ERROR_DATA_OUT_OF_RANGE;
		}

		mode_state->range_index = params->range_index;
		mode_state->auto_range = params->auto_range;
		mode_state->resolution_index = params->resolution_index;
	}

	dm_multimeter_state.mode = mode;
	dm_multimeter_state.mode_initialized = TRUE;

	return SCPIMM_ERROR_OK;
}

/***************************************************************
 * Interface functions
 **************************************************************/

const char* dm_read_entire_output_buffer() {
	// copy all the data from output buffer to intermediate buffer
	memcpy(out_buffer_read, out_buffer, out_buffer_counter);
	out_buffer_read[out_buffer_counter] = '\0';

	// reset output buffer pointer
	out_buffer_counter = 0;

	return out_buffer_read;
}

void dm_reset_counters() {
	memset(&dm_counters, 0, sizeof(dm_counters));
}

void dm_reset_args() {
	memset(&dm_args, 0xfe, sizeof(dm_args));
}

double dm_measurement_func_const(uint32_t time) {
	const dm_mode_state_t* const mode_state = get_mode_state(dm_multimeter_state.mode);

	(void) time;

	return 0.5 * RANGES[mode_state ? mode_state->range_index : 0];
}

/***************************************************************
 * Multimeter interface
 **************************************************************/

static int16_t dm_setup() {
	dm_counters.setup++;

	if (!measure_thread_created) {
		int err;

		err = sem_init(&measure_sem, 0, 0);
	    if (err) {
	        printf("Could not initialize a semaphore\n");
	        return err;
	    }

	    err = pthread_create(&measure_thread, NULL, measure_thread_routine, NULL);
		if (err) {
			printf("Error creating measurement thread");
			exit(err);
		}
		measure_thread_created = TRUE;
	}

	if (!trigger_thread_created) {
		int err;

	    err = pthread_create(&trigger_thread, NULL, trigger_thread_routine, NULL);
		if (err) {
			printf("Error creating trigger thread");
			exit(err);
		}
		trigger_thread_created = TRUE;
	}

	return SCPIMM_ERROR_OK;
}

static void reset_mode_state(dm_mode_state_t* dest) {
	memset(dest, 0, sizeof(*dest));
	dest->auto_range = TRUE;
}

static int16_t dm_reset() {
	dm_counters.reset++;

	dm_reset_args();
	memset(&dm_returns, 0, sizeof(dm_returns));

	dm_multimeter_state.interrrupt_disable_counter = 0;
	dm_multimeter_state.measurement_failure_counter = 0;
	dm_multimeter_state.terminal_state = SCPIMM_TERM_FRONT;
	dm_multimeter_state.input_impedance_auto_state = FALSE;
	dm_multimeter_state.zero_auto = TRUE;
	dm_multimeter_state.zero_auto_once = FALSE;
	dm_multimeter_state.remote = TRUE;
	dm_multimeter_state.lock = FALSE;

	reset_mode_state(&dm_multimeter_state.mode_states.dcv);
	reset_mode_state(&dm_multimeter_state.mode_states.dcv_ratio);
	reset_mode_state(&dm_multimeter_state.mode_states.acv);
	reset_mode_state(&dm_multimeter_state.mode_states.acv_ratio);
	reset_mode_state(&dm_multimeter_state.mode_states.dcc);
	reset_mode_state(&dm_multimeter_state.mode_states.acc);
	reset_mode_state(&dm_multimeter_state.mode_states.resistance);
	reset_mode_state(&dm_multimeter_state.mode_states.fresistance);

	dm_multimeter_config.measurement_type = DM_MEASUREMENT_TYPE_ASYNC;
	dm_multimeter_config.measurement_func = dm_measurement_func_const;
	dm_multimeter_config.measurement_duration = DEFAULT_MEAS_DURATION;

	out_buffer_counter = 0;

	set_mode_impl(SCPIMM_MODE_DCV, NULL);

	return SCPIMM_ERROR_OK;
}

static int16_t dm_set_mode(const scpimm_mode_t mode, const scpimm_mode_params_t* const params) {
	dm_counters.set_mode++;

	/* store function arguments for later analysis */
	dm_args.set_mode.mode = mode;
	if (params) {
		dm_args.set_mode.params = *params;
		dm_args.set_mode.params_is_null = FALSE;
	} else {
		dm_args.set_mode.params_is_null = TRUE;
	}
	/* */

	return set_mode_impl(mode, params);
}

static int16_t dm_get_mode(scpimm_mode_t* mode) {
	dm_counters.get_mode++;

	if (!dm_multimeter_state.mode_initialized) {
		return SCPIMM_ERROR_INTERNAL;
	}

	if (mode) {
		*mode = dm_multimeter_state.mode;
	}

	return SCPIMM_ERROR_OK;
}

static int16_t dm_get_allowed_resolutions(scpimm_mode_t mode, size_t range_index, const double** resolutions) {
	int16_t err;

	dm_counters.get_allowed_resolutions++;

	/* store function arguments for later analysis */
	dm_args.get_allowed_resolutions.mode = mode;
	dm_args.get_allowed_resolutions.range_index = range_index;
	if (resolutions) {
		dm_args.get_allowed_resolutions.resolutions = *resolutions;
		dm_args.get_allowed_resolutions.resolutions_is_null = FALSE;
	} else {
		dm_args.get_allowed_resolutions.resolutions_is_null = TRUE;
	}
	/* */

	if (SCPIMM_ERROR_OK != (err = validate_mode(mode))) {
		return err;
	}

	if (range_index >= sizeof(RANGES) / sizeof(RANGES[0])) {
		return SCPIMM_ERROR_DATA_OUT_OF_RANGE;
	}

	if (resolutions) {
		*resolutions = RESOLUTIONS[range_index];
	}

	return SCPIMM_ERROR_OK;
}

static int16_t dm_start_measure() {
	dm_counters.start_measure++;

	if (dm_multimeter_state.measurement_failure_counter) {
		if (1 == dm_multimeter_state.measurement_failure_counter--) {
			// emulate internal multimeter error when "measurement is ready" signal is lost
			return SCPIMM_ERROR_OK;
		}
	}

	if (dm_multimeter_state.zero_auto_once) {
		dm_multimeter_state.zero_auto = FALSE;
		dm_multimeter_state.zero_auto_once = FALSE;
	}

	switch (dm_multimeter_config.measurement_type) {
	case DM_MEASUREMENT_TYPE_ASYNC:
		sem_post(&measure_sem);
		break;

	case DM_MEASUREMENT_TYPE_SYNC:
		do_measurement();
		break;
	}
	return SCPIMM_ERROR_OK;
}

static size_t dm_send(const uint8_t* data, const size_t len) {
	memcpy(out_buffer + out_buffer_counter, (const char*) data, len);
	out_buffer_counter += len;
	return len;
}

static int16_t dm_get_milliseconds(uint32_t* const tm) {
	if (tm) {
		*tm = get_milliseconds();
	}
	return SCPIMM_ERROR_OK;
}

static int16_t dm_set_interrupt_status(const scpi_bool_t disabled) {
	dm_counters.set_interrupt_status++;

	if (disabled) {
		dm_multimeter_state.interrrupt_disable_counter++;
	} else {
		dm_multimeter_state.interrrupt_disable_counter--;
	}

	return SCPIMM_ERROR_OK;
}

static int16_t dm_get_global_bool_param(const scpimm_bool_param_t param, scpi_bool_t* const value) {
	scpi_bool_t res = FALSE;

	dm_counters.get_global_bool_param++;

	dm_prev_args = dm_args;
	dm_args.get_global_bool_param.param = param;
	dm_args.get_global_bool_param.value_is_null = !value;

	switch (param) {
	case SCPIMM_PARAM_INPUT_IMPEDANCE_AUTO:
		res = dm_multimeter_state.input_impedance_auto_state;
		break;

	case SCPIMM_PARAM_ZERO_AUTO:
		res = dm_multimeter_state.zero_auto;
		break;

	case SCPIMM_PARAM_ZERO_AUTO_ONCE:
		res = dm_multimeter_state.zero_auto_once;
		break;

	case SCPIMM_PARAM_REMOTE:
		res = dm_multimeter_state.remote;
		break;

	case SCPIMM_PARAM_LOCK:
		res = dm_multimeter_state.lock;
		break;

	case SCPIMM_PARAM_RANGE_AUTO:
		return SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE;
	}

	if (value) {
		*value = res;
	}

	return dm_returns.get_global_bool_param;
}

static int16_t dm_set_global_bool_param(const scpimm_bool_param_t param, const scpi_bool_t value) {
	dm_counters.set_global_bool_param++;

	dm_prev_args = dm_args;
	dm_args.set_global_bool_param.param = param;
	dm_args.set_global_bool_param.value = value;

	if (SCPIMM_ERROR_OK != dm_returns.set_global_bool_param) {
		return dm_returns.set_global_bool_param;
	}

	switch (param) {
	case SCPIMM_PARAM_INPUT_IMPEDANCE_AUTO:
		dm_multimeter_state.input_impedance_auto_state = value;
		break;

	case SCPIMM_PARAM_ZERO_AUTO:
		dm_multimeter_state.zero_auto = value;
		break;

	case SCPIMM_PARAM_ZERO_AUTO_ONCE:
		dm_multimeter_state.zero_auto_once = value;
		break;

	case SCPIMM_PARAM_REMOTE:
		dm_multimeter_state.remote = value;
		break;

	case SCPIMM_PARAM_LOCK:
		dm_multimeter_state.lock = value;
		break;

	case SCPIMM_PARAM_RANGE_AUTO:
		return SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE;
	}

	return SCPIMM_ERROR_OK;
}

static int16_t dm_get_bool_param(const scpimm_mode_t mode, const scpimm_bool_param_t param, scpi_bool_t* const value) {
	const dm_mode_state_t* const mode_state = get_mode_state(mode);
	scpi_bool_t v = FALSE;

	dm_counters.get_bool_param++;

	dm_args.get_bool_param.mode = mode;
	dm_args.get_bool_param.param = param;
	dm_args.get_bool_param.value_is_null = !value;

	if (!mode_state) {
		return SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE;
	}

	switch (param) {
	case SCPIMM_PARAM_RANGE_AUTO:
		v = mode_state->auto_range;
		break;

	case SCPIMM_PARAM_ZERO_AUTO:
	case SCPIMM_PARAM_ZERO_AUTO_ONCE:
	case SCPIMM_PARAM_INPUT_IMPEDANCE_AUTO:
	case SCPIMM_PARAM_REMOTE:
	case SCPIMM_PARAM_LOCK:
		return SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE;
	}

	if (value) {
		*value = v;
	}

	return dm_returns.get_bool_param;
}

static int16_t dm_set_bool_param(const scpimm_mode_t mode, const scpimm_bool_param_t param, const scpi_bool_t value) {
	dm_mode_state_t* const mode_state = get_mode_state(mode);

	dm_counters.set_bool_param++;

	dm_args.set_bool_param.mode = mode;
	dm_args.set_bool_param.param = param;
	dm_args.set_bool_param.value = value;

	if (!mode_state) {
		return SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE;
	}

	switch (param) {
	case SCPIMM_PARAM_RANGE_AUTO:
		mode_state->auto_range = value;
		break;

	case SCPIMM_PARAM_ZERO_AUTO:
	case SCPIMM_PARAM_ZERO_AUTO_ONCE:
	case SCPIMM_PARAM_INPUT_IMPEDANCE_AUTO:
	case SCPIMM_PARAM_REMOTE:
	case SCPIMM_PARAM_LOCK:
		return SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE;
	}

	return dm_returns.set_bool_param;
}

static int16_t dm_get_numeric_param_values(const scpimm_mode_t mode, const scpimm_numeric_param_t param, const double** values) {
	int16_t err;
	const double* v = NULL;

	dm_counters.get_numeric_param_values++;

	dm_args.get_numeric_param_values.mode = mode;
	dm_args.get_numeric_param_values.param = param;
	dm_args.get_numeric_param_values.values_is_null = !values;

	if (SCPIMM_ERROR_OK != (err = validate_mode(mode))) {
		return err;
	}

	switch (param) {
	case SCPIMM_PARAM_RANGE:
		v = RANGES;
		break;

	case SCPIMM_PARAM_RANGE_OVERRUN:
		v = OVERRUNS;
		break;

	case SCPIMM_PARAM_NPLC:
		v = NPLCS;
		break;

	case SCPIMM_PARAM_RESOLUTION:
		return SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE;
	}

	if (!v) {
		return SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE;
	}

	if (values) {
		*values = v;
	}

	return dm_returns.get_numeric_param_values;
}

static int16_t dm_get_numeric_param(scpimm_mode_t mode, scpimm_numeric_param_t param, size_t* value_index) {
	const dm_mode_state_t* const mode_state = get_mode_state(mode);
	size_t v = SIZE_MAX;

	dm_counters.get_numeric_param++;

	dm_args.get_numeric_param.mode = mode;
	dm_args.get_numeric_param.param = param;
	dm_args.get_numeric_param.value_is_null = !value_index;

	if (!mode_state) {
		return SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE;
	}

	switch (param) {
	case SCPIMM_PARAM_RANGE:
		v = mode_state->range_index;
		break;

	case SCPIMM_PARAM_RESOLUTION:
		v = mode_state->resolution_index;
		break;

	case SCPIMM_PARAM_NPLC:
		v = mode_state->nplc_index;
		break;

	case SCPIMM_PARAM_RANGE_OVERRUN:
		return SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE;
	}

	if (value_index) {
		*value_index = v;
	}

	return dm_returns.get_numeric_param;
}

static int16_t dm_set_numeric_param(scpimm_mode_t mode, scpimm_numeric_param_t param, size_t value_index) {
	dm_mode_state_t* const mode_state = get_mode_state(mode);

	dm_counters.set_numeric_param++;

	dm_args.set_numeric_param.mode = mode;
	dm_args.set_numeric_param.param = param;
	dm_args.set_numeric_param.value_index = value_index;

	if (!mode_state) {
		return SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE;
	}

	switch (param) {
	case SCPIMM_PARAM_RANGE:
		mode_state->range_index = value_index;
		break;

	case SCPIMM_PARAM_RESOLUTION:
		mode_state->resolution_index = value_index;
		break;

	case SCPIMM_PARAM_NPLC:
		mode_state->nplc_index = value_index;
		break;

	case SCPIMM_PARAM_RANGE_OVERRUN:
		return SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE;
	}

	return dm_returns.set_numeric_param;
}

static int16_t dm_beep() {
	dm_counters.beep++;
	return SCPIMM_ERROR_OK;
}

static int16_t dm_get_input_terminal(scpimm_terminal_state_t* term) {
	if (term) {
		*term = dm_multimeter_state.terminal_state;
	}
	return dm_returns.get_input_terminal;
}

static int16_t dm_display_text(const char* txt) {
	dm_counters.display_text++;
	dm_args.display_text.txt = txt;

	strncpy(dm_display, txt, sizeof(dm_display) - 1);
	dm_display[sizeof(dm_display) - 1] = '\0';

	return SCPIMM_ERROR_OK;
}

static int16_t dm_test() {
	dm_counters.test++;
	return dm_returns.test;
}
