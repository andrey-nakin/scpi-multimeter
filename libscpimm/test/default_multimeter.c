#include <string.h>
#include "default_multimeter.h"

#define MIN_RANGE 1.0e-1
#define MAX_RANGE 1.0e3

#define BEST_RESOLUTION	1.0e-6
#define	WORST_RESOLUTION 1.0e-3

static scpimm_mode_t dm_supported_modes(void);
static int16_t dm_set_mode(scpimm_mode_t mode, const scpimm_mode_params_t* const params);
static int16_t dm_get_mode(scpimm_mode_t* mode, scpimm_mode_params_t* const params);
static int16_t dm_get_possible_range(scpimm_mode_t mode, double* min_range, double* max_range);
static int16_t dm_get_possible_resolution(scpimm_mode_t mode, double range, double* min_resolution, double* max_resolution);
static bool_t dm_start_measure();
static size_t dm_send(const uint8_t* buf, size_t len);

/***************************************************************
 * Global variables
 **************************************************************/

dm_multimeter_state_t dm_multimeter_state;
dm_set_mode_args_t dm_set_mode_last_args;
dm_get_possible_range_args_t dm_get_possible_range_last_args;
dm_get_possible_resolution_args_t dm_get_possible_resolution_last_args;
scpimm_interface_t dm_interface = {
		.supported_modes = dm_supported_modes,
		.set_mode = dm_set_mode,
		.get_mode = dm_get_mode,
		.get_possible_range = dm_get_possible_range,
		.get_possible_resolution = dm_get_possible_resolution,
		.start_measure = dm_start_measure,
		.send = dm_send
};
dm_counters_t dm_counters;

static char inbuffer[1024];
static char* inpuffer_pos = inbuffer;

/***************************************************************
 * Private functions
 **************************************************************/

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

/***************************************************************
 * Multimeter interface
 **************************************************************/

static scpimm_mode_t dm_supported_modes(void) {
	return SCPIMM_MODE_DCV | SCPIMM_MODE_DCV_RATIO | SCPIMM_MODE_ACV | SCPIMM_MODE_DCC
			| SCPIMM_MODE_ACC | SCPIMM_MODE_RESISTANCE_2W | SCPIMM_MODE_RESISTANCE_4W
			| SCPIMM_MODE_FREQUENCY | SCPIMM_MODE_PERIOD | SCPIMM_MODE_CONTINUITY
			| SCPIMM_MODE_DIODE;
}

static int16_t dm_set_mode(const scpimm_mode_t mode, const scpimm_mode_params_t* const params) {

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
		break;

	default:
		return SCPI_ERROR_ILLEGAL_PARAMETER_VALUE;
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

static int16_t dm_get_possible_range(const scpimm_mode_t mode, double* const min_range, double* const max_range) {

	/* store function arguments for later analysis */
	dm_get_possible_range_last_args.mode = mode;
	if (min_range) {
		dm_get_possible_range_last_args.min_range = *min_range;
		dm_get_possible_range_last_args.min_range_is_null = FALSE;
	} else {
		dm_get_possible_range_last_args.min_range_is_null = TRUE;
	}
	if (max_range) {
		dm_get_possible_range_last_args.max_range = *max_range;
		dm_get_possible_range_last_args.max_range_is_null = FALSE;
	} else {
		dm_get_possible_range_last_args.max_range_is_null = TRUE;
	}
	/* */

	if (min_range) {
		*min_range = MIN_RANGE;
	}

	if (max_range) {
		*max_range = MAX_RANGE;
	}

	return SCPI_ERROR_OK;
}

static int16_t dm_get_possible_resolution(const scpimm_mode_t mode, const double range, double* const min_resolution, double* const max_resolution) {

	/* store function arguments for later analysis */
	dm_get_possible_resolution_last_args.mode = mode;
	dm_get_possible_resolution_last_args.range = range;
	if (min_resolution) {
		dm_get_possible_resolution_last_args.min_resolution = *min_resolution;
		dm_get_possible_resolution_last_args.min_resolution_is_null = FALSE;
	} else {
		dm_get_possible_resolution_last_args.min_resolution_is_null = TRUE;
	}
	if (max_resolution) {
		dm_get_possible_resolution_last_args.max_resolution = *max_resolution;
		dm_get_possible_resolution_last_args.max_resolution_is_null = FALSE;
	} else {
		dm_get_possible_resolution_last_args.max_resolution_is_null = TRUE;
	}
	/* */

	if (min_resolution) {
		*min_resolution = range * BEST_RESOLUTION;
	}

	if (max_resolution) {
		*max_resolution = range * WORST_RESOLUTION;
	}

	return SCPI_ERROR_OK;
}

static bool_t dm_start_measure() {
	return TRUE;
}

static size_t dm_send(const uint8_t* data, size_t len) {
	memcpy(inpuffer_pos, (const char*) data, len);
	inpuffer_pos += len;
	*inpuffer_pos = '\0';
	return len;
}
