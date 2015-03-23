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
static int16_t dm_get_mode(scpimm_mode_t* mode, scpimm_mode_params_t* const params);
static int16_t dm_get_allowed_ranges(scpimm_mode_t mode, const double** const ranges, const double** const overruns);
static int16_t dm_get_allowed_resolutions(scpimm_mode_t mode, size_t range_index, const double** resolutions);
static int16_t dm_start_measure();
static size_t dm_send(const uint8_t* buf, size_t len);
static int16_t dm_get_milliseconds(uint32_t* tm);
static int16_t dm_sleep_milliseconds(uint32_t);
static int16_t dm_set_interrupt_status(scpi_bool_t disabled);
static int16_t dm_remote(scpi_bool_t remote, scpi_bool_t lock);
static int16_t dm_beep();
static int16_t dm_display_text(const char* txt);

/***************************************************************
 * Global variables
 **************************************************************/

dm_multimeter_state_t dm_multimeter_state;
char dm_display[SCPIMM_DISPLAY_LEN + 1];

dm_set_mode_args_t dm_set_mode_last_args;
dm_get_allowed_ranges_args_t dm_get_allowed_ranges_last_args;
dm_get_allowed_resolutions_args_t dm_get_allowed_resolutions_last_args;
dm_remote_args_t dm_remote_args;
dm_display_text_args_t dm_display_text_args;

scpimm_interface_t dm_interface = {
		.setup = dm_setup,
		.reset = dm_reset,
		.set_mode = dm_set_mode,
		.get_mode = dm_get_mode,
		.get_allowed_ranges = dm_get_allowed_ranges,
		.get_allowed_resolutions = dm_get_allowed_resolutions,
		.start_measure = dm_start_measure,
		.send = dm_send,
		.get_milliseconds = dm_get_milliseconds,
		.sleep_milliseconds = dm_sleep_milliseconds,
		.set_interrupt_status = dm_set_interrupt_status,
		.remote = dm_remote,
		.beep = dm_beep,
		.display_text = dm_display_text
};
dm_counters_t dm_counters;
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

static pthread_t measure_thread, trigger_thread;
static scpi_bool_t measure_thread_created = FALSE;
static scpi_bool_t trigger_thread_created = FALSE;
static sem_t measure_sem;

/***************************************************************
 * Private functions
 **************************************************************/

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

		dm_sleep_milliseconds(dm_multimeter_config.measurement_duration);	//	emulate real measurement delay
		do_measurement();
	}

	return NULL;
}

static void* trigger_thread_routine(void* args) {
	(void) args;	//	suppress warning

	while (TRUE) {
		dm_sleep_milliseconds(100);
		SCPIMM_external_trigger();
	}

	return NULL;
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
#define RESET_ARG(v) memset(&v, 0xfe, sizeof(v))

	RESET_ARG(dm_set_mode_last_args);
	RESET_ARG(dm_get_allowed_ranges_last_args);
	RESET_ARG(dm_get_allowed_resolutions_last_args);
	RESET_ARG(dm_remote_args);
	RESET_ARG(dm_display_text_args);
	RESET_ARG(dm_display);
}

double dm_measurement_func_const(uint32_t time) {
	(void) time;

	return dm_multimeter_state.mode_initialized && dm_multimeter_state.mode_params_initialized ?
			0.5 * RANGES[dm_multimeter_state.mode_params.range_index]
			: 0.0;
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

	return SCPI_ERROR_OK;
}

static int16_t dm_reset() {
	dm_counters.reset++;

	dm_reset_args();

	dm_multimeter_state.interrrupt_disable_counter = 0;
	dm_multimeter_state.measurement_failure_counter = 0;

	dm_multimeter_config.measurement_type = DM_MEASUREMENT_TYPE_ASYNC;
	dm_multimeter_config.measurement_func = dm_measurement_func_const;
	dm_multimeter_config.measurement_duration = DEFAULT_MEAS_DURATION;

	out_buffer_counter = 0;

	return SCPI_ERROR_OK;
}

static int16_t dm_validate_mode(const scpimm_mode_t mode) {
	switch (mode) {
	case SCPIMM_MODE_DCV:
	case SCPIMM_MODE_DCV_RATIO:
	case SCPIMM_MODE_ACV:
	case SCPIMM_MODE_DCC:
	case SCPIMM_MODE_ACC:
	case SCPIMM_MODE_RESISTANCE_2W:
	case SCPIMM_MODE_RESISTANCE_4W:
		return SCPI_ERROR_OK;

	default:
		return SCPI_ERROR_ILLEGAL_PARAMETER_VALUE;
	}
}

static size_t max_resolution_index(size_t range_index) {
	const double* resolutions = RESOLUTIONS[range_index];
	size_t resolution_index = 0;

	for (; resolutions[resolution_index] >= 0.0; resolution_index++) {};
	return --resolution_index;
}

static int16_t dm_set_mode(const scpimm_mode_t mode, const scpimm_mode_params_t* const params) {
	int16_t err;

	dm_counters.set_mode++;

	/* store function arguments for later analysis */
	dm_set_mode_last_args.mode = mode;
	if (params) {
		dm_set_mode_last_args.params = *params;
		dm_set_mode_last_args.params_is_null = FALSE;
	} else {
		dm_set_mode_last_args.params_is_null = TRUE;
	}
	/* */

	if (SCPI_ERROR_OK != (err = dm_validate_mode(mode))) {
		return err;
	}

	if (params) {
		if (sizeof(RANGES) / sizeof(RANGES[0]) - 1 <= params->range_index) {
			return SCPI_ERROR_DATA_OUT_OF_RANGE;
		}
		if (max_resolution_index(params->range_index) < params->resolution_index) {
			return SCPI_ERROR_DATA_OUT_OF_RANGE;
		}

		dm_multimeter_state.mode_params = *params;
		dm_multimeter_state.mode_params_initialized = TRUE;
	}

	dm_multimeter_state.mode = mode;
	dm_multimeter_state.mode_initialized = TRUE;

	return SCPI_ERROR_OK;
}

static int16_t dm_get_mode(scpimm_mode_t* mode, scpimm_mode_params_t* const params) {
	dm_counters.get_mode++;

	if (mode) {
		if (!dm_multimeter_state.mode_initialized) {
			return SCPI_ERROR_INTERNAL_START;
		}
		*mode = dm_multimeter_state.mode;
	}

	if (params) {
		if (!dm_multimeter_state.mode_params_initialized) {
			return SCPI_ERROR_INTERNAL_START;
		}
		*params = dm_multimeter_state.mode_params;
	}

	return SCPI_ERROR_OK;
}

static int16_t dm_get_allowed_ranges(scpimm_mode_t mode, const double** const ranges, const double** const overruns) {
	int16_t err;

	dm_counters.get_allowed_ranges++;

	/* store function arguments for later analysis */
	dm_get_allowed_ranges_last_args.mode = mode;
	if (ranges) {
		dm_get_allowed_ranges_last_args.ranges = *ranges;
		dm_get_allowed_ranges_last_args.ranges_is_null = FALSE;
	} else {
		dm_get_allowed_ranges_last_args.ranges_is_null = TRUE;
	}
	if (overruns) {
		dm_get_allowed_ranges_last_args.overruns = *overruns;
		dm_get_allowed_ranges_last_args.overruns_is_null = FALSE;
	} else {
		dm_get_allowed_ranges_last_args.overruns_is_null = TRUE;
	}
	/* */

	if (SCPI_ERROR_OK != (err = dm_validate_mode(mode))) {
		return err;
	}

	if (ranges) {
		*ranges = RANGES;
	}

	if (overruns) {
		*overruns = OVERRUNS;
	}

	return SCPI_ERROR_OK;
}

static int16_t dm_get_allowed_resolutions(scpimm_mode_t mode, size_t range_index, const double** resolutions) {
	int16_t err;

	dm_counters.get_allowed_resolutions++;

	/* store function arguments for later analysis */
	dm_get_allowed_resolutions_last_args.mode = mode;
	dm_get_allowed_resolutions_last_args.range_index = range_index;
	if (resolutions) {
		dm_get_allowed_resolutions_last_args.resolutions = *resolutions;
		dm_get_allowed_resolutions_last_args.resolutions_is_null = FALSE;
	} else {
		dm_get_allowed_resolutions_last_args.resolutions_is_null = TRUE;
	}
	/* */

	if (SCPI_ERROR_OK != (err = dm_validate_mode(mode))) {
		return err;
	}

	if (range_index >= sizeof(RANGES) / sizeof(RANGES[0])) {
		return SCPI_ERROR_DATA_OUT_OF_RANGE;
	}

	if (resolutions) {
		*resolutions = RESOLUTIONS[range_index];
	}

	return SCPI_ERROR_OK;
}

static int16_t dm_start_measure() {
	dm_counters.start_measure++;

	if (dm_multimeter_state.measurement_failure_counter) {
		if (1 == dm_multimeter_state.measurement_failure_counter--) {
			// emulate internal multimeter error when "measurement is ready" signal is lost
			return SCPI_ERROR_OK;
		}
	}

	switch (dm_multimeter_config.measurement_type) {
	case DM_MEASUREMENT_TYPE_ASYNC:
		sem_post(&measure_sem);
		break;

	case DM_MEASUREMENT_TYPE_SYNC:
		do_measurement();
		break;
	}
	return SCPI_ERROR_OK;
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
	return SCPI_ERROR_OK;
}

static int16_t dm_sleep_milliseconds(const uint32_t ms) {
	struct timespec delay;

	if (ms > 999) {
		delay.tv_sec = ms / 1000;
		delay.tv_nsec = ms % 1000 * 1000000;
	} else {
		delay.tv_sec = 0;
		delay.tv_nsec = ms * 1000000;
	}
	nanosleep(&delay, NULL);
	return SCPI_ERROR_OK;
}

static int16_t dm_set_interrupt_status(const scpi_bool_t disabled) {
	dm_counters.set_interrupt_status++;

	if (disabled) {
		dm_multimeter_state.interrrupt_disable_counter++;
	} else {
		dm_multimeter_state.interrrupt_disable_counter--;
	}

	return SCPI_ERROR_OK;
}

static int16_t dm_remote(scpi_bool_t remote, scpi_bool_t lock) {
	dm_counters.remote++;
	dm_remote_args.remote = remote;
	dm_remote_args.lock = lock;

	return SCPI_ERROR_OK;
}

static int16_t dm_beep() {
	dm_counters.beep++;
	return SCPI_ERROR_OK;
}

static int16_t dm_display_text(const char* txt) {
	dm_counters.display_text++;
	dm_display_text_args.txt = txt;

	strncpy(dm_display, txt, sizeof(dm_display) - 1);
	dm_display[sizeof(dm_display) - 1] = '\0';

	return SCPI_ERROR_OK;
}
