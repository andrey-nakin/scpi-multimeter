#include <string.h>
#include "default_multimeter.h"

static scpimm_mode_t dm_supported_modes(void);
static int16_t dm_set_mode(scpimm_mode_t mode, const scpimm_mode_params_t* const params);
static int16_t dm_get_mode(scpimm_mode_t* mode, scpimm_mode_params_t* const params);
static int16_t dm_get_allowed_ranges(scpimm_mode_t mode, const double** const ranges, const double** const overruns);
static int16_t dm_get_allowed_resolutions(scpimm_mode_t mode, size_t range_index, const double** resolutions);
static bool_t dm_start_measure();
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
		.set_mode = dm_set_mode,
		.get_mode = dm_get_mode,
		.get_allowed_ranges = dm_get_allowed_ranges,
		.get_allowed_resolutions = dm_get_allowed_resolutions,
		.start_measure = dm_start_measure,
		.send = dm_send
};
dm_counters_t dm_counters;

static char inbuffer[1024];
static char* inpuffer_pos = inbuffer;

static const double RANGES[] =   {0.1, 10.0, 100.0, 1000.0, -1.0};
static const double OVERRUNS[] = {1.2, 1.2,  1.2,   1.2,    -1.0};

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

static int16_t dm_get_allowed_ranges(scpimm_mode_t mode, const double** const ranges, const double** const overruns) {

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

	if (ranges) {
		*ranges = RANGES;
	}

	if (overruns) {
		*overruns = OVERRUNS;
	}

	return SCPI_ERROR_OK;
}

static int16_t dm_get_allowed_resolutions(scpimm_mode_t mode, size_t range_index, const double** resolutions) {

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

	if (resolutions) {
		static double res[5];
		const double* ranges;

		dm_get_allowed_ranges(mode, &ranges, NULL);
		res[0] = ranges[range_index] * 1.0e-6;
		res[1] = ranges[range_index] * 1.0e-5;
		res[2] = ranges[range_index] * 1.0e-4;
		res[3] = ranges[range_index] * 1.0e-3;
		res[4] = -1.0;
		*resolutions = res;
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
