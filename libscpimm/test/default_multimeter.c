#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "default_multimeter.h"

static scpimm_mode_t dm_supported_modes(void);
static int16_t dm_reset();
static int16_t dm_set_mode(scpimm_mode_t mode, const scpimm_mode_params_t* const params);
static int16_t dm_get_mode(scpimm_mode_t* mode, scpimm_mode_params_t* const params);
static int16_t dm_get_allowed_ranges(scpimm_mode_t mode, const double** const ranges, const double** const overruns);
static int16_t dm_get_allowed_resolutions(scpimm_mode_t mode, size_t range_index, const double** resolutions);
static int16_t dm_start_measure();
static size_t dm_send(const uint8_t* buf, size_t len);

/***************************************************************
 * Global variables
 **************************************************************/

dm_multimeter_state_t dm_multimeter_state;
dm_set_mode_args_t dm_set_mode_last_args;
dm_get_allowed_ranges_args_t dm_get_allowed_ranges_last_args;
dm_get_allowed_resolutions_args_t dm_get_allowed_resolutions_last_args;
scpimm_interface_t dm_interface = {
		.supported_modes = dm_supported_modes,
		.reset = dm_reset,
		.set_mode = dm_set_mode,
		.get_mode = dm_get_mode,
		.get_allowed_ranges = dm_get_allowed_ranges,
		.get_allowed_resolutions = dm_get_allowed_resolutions,
		.start_measure = dm_start_measure,
		.send = dm_send
};
dm_counters_t dm_counters;
dm_measuremenet_func_t dm_measuremenet_func = dm_measurement_func_const;

static char inbuffer[1024];
static char* inpuffer_pos = inbuffer;

static const double RANGES[] =   {0.1, 1.0, 10.0, 100.0, 1000.0, -1.0};
static const double OVERRUNS[] = {1.2, 1.2, 1.2,  1.2,   1.2,    -1.0};
static const double RESOLUTIONS[][5] = {
		{0.1e-6, 0.1e-5, 0.1e-4, 0.1e-3, -1.0},
		{1.0e-6, 1.0e-5, 1.0e-4, 1.0e-3, -1.0},
		{10e-6, 10e-5, 10e-4, 10e-3, -1.0},
		{100e-6, 100e-5, 100e-4, 100e-3, -1.0},
		{1000e-6, 1000e-5, 1000e-4, 1000e-3, -1.0}
};

static pthread_t measure_thread;
static bool_t measure_thread_created = FALSE;
static sem_t measure_sem;

/***************************************************************
 * Private functions
 **************************************************************/

static void do_measurement() {
	scpi_number_t number = {0.0, SCPI_UNIT_NONE, SCPI_NUM_NUMBER};
	number.value = dm_measuremenet_func(0L);	// TODO add time
	SCPIMM_read_value(&number);
}

static void* measure_thread_routine(void* args) {
	struct timespec delay = {0, 500 * 1000000};

	(void) args;	//	suppress warning

	while (TRUE) {
		sem_wait(&measure_sem);
		printf("measure_thread_routine semaphore released\n");

		nanosleep(&delay, NULL);	//	emulate real measurement delay
		do_measurement();
	}

	printf("measure_thread_routine finished\n");
	return NULL;
}

/***************************************************************
 * Interface functions
 **************************************************************/

void dm_init_in_buffer() {
	inpuffer_pos = inbuffer;
	*inpuffer_pos = '\0';
}

char* dm_output_buffer() {
	return inbuffer;
}

void dm_reset_counters() {
	memset(&dm_counters, 0, sizeof(dm_counters));
}

double dm_measurement_func_const(long time) {
	(void) time;

	return dm_multimeter_state.mode_initialized && dm_multimeter_state.mode_params_initialized ?
			0.5 * RANGES[dm_multimeter_state.mode_params.range_index]
			: 0.0;
}

/***************************************************************
 * Multimeter interface
 **************************************************************/

static scpimm_mode_t dm_supported_modes(void) {
	return SCPIMM_MODE_DCV | SCPIMM_MODE_DCV_RATIO | SCPIMM_MODE_ACV | SCPIMM_MODE_DCC
			| SCPIMM_MODE_ACC | SCPIMM_MODE_RESISTANCE_2W | SCPIMM_MODE_RESISTANCE_4W
			| SCPIMM_MODE_FREQUENCY | SCPIMM_MODE_PERIOD | SCPIMM_MODE_CONTINUITY
			| SCPIMM_MODE_DIODE;
}

static int16_t dm_reset() {
	if (!measure_thread_created) {
		int err;

		err = sem_init(&measure_sem, 0, 0);
	    if (err) {
	        printf("Could not initialize a semaphore\n");
	        return err;
	    }

	    err = pthread_create(&measure_thread, NULL, measure_thread_routine, NULL);
		if (err) {
			printf("Error creating thread");
			exit(err);
		}
		measure_thread_created = TRUE;
	}

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
	case SCPIMM_MODE_FREQUENCY:
	case SCPIMM_MODE_PERIOD:
	case SCPIMM_MODE_CONTINUITY:
	case SCPIMM_MODE_DIODE:
		return SCPI_ERROR_OK;

	default:
		return SCPI_ERROR_ILLEGAL_PARAMETER_VALUE;
	}
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

	dm_multimeter_state.mode = mode;
	dm_multimeter_state.mode_initialized = TRUE;
	if (params) {
		dm_multimeter_state.mode_params = *params;
		dm_multimeter_state.mode_params_initialized = TRUE;
	}

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
	sem_post(&measure_sem);
	return SCPI_ERROR_OK;
}

static size_t dm_send(const uint8_t* data, size_t len) {
	memcpy(inpuffer_pos, (const char*) data, len);
	inpuffer_pos += len;
	*inpuffer_pos = '\0';
	return len;
}
