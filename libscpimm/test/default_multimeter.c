#include "default_multimeter.h"

#define MIN_RANGE 1.0e-1
#define MAX_RANGE 1.0e3

#define BEST_RESOLUTION	1.0e-6
#define	WORST_RESOLUTION 1.0e-3

dm_multimeter_state_t dm_multimeter_state;
dm_set_mode_args_t dm_set_mode_last_args;
dm_get_possible_range_args_t dm_get_possible_range_last_args;
dm_get_possible_resolution_args_t dm_get_possible_resolution_last_args;

int16_t dm_set_mode(const scpimm_mode_t mode, const scpimm_mode_params_t* const params) {
	//double new_range = dm_multimeter_state.range;
	//double new_resolution = dm_multimeter_state.resolution;
	//bool_t new_auto_range = dm_multimeter_state.auto_range;
	//int16_t err = SCPI_ERROR_OK;
	//double min_value, max_value;

	/* store function arguments for later analysis */
	dm_set_mode_last_args.mode = mode;
	if (params) {
		dm_set_mode_last_args.params = *params;
		dm_set_mode_last_args.params_is_null = FALSE;
	} else {
		dm_set_mode_last_args.params_is_null = TRUE;
	}
	/* */

	dm_multimeter_state.mode = mode;
	if (params) {
		dm_multimeter_state.mode_params = *params;

/*		switch (params->range.type) {
		case SCPI_NUM_NUMBER:
			CHECK_ERROR(dm_get_possible_range(mode, &min_value, &max_value));
			if (params->range.value < min_value) {
				new_range = min_value;
			} else if (params->range.value > max_value * INCREASE_DELTA) {
				return SCPI_ERROR_DATA_OUT_OF_RANGE;
			} else {
				new_range = params->range.value;
			}
			break;

		case SCPI_NUM_MIN:
			CHECK_ERROR(dm_get_possible_range(mode, &new_range, NULL));
			break;

		case SCPI_NUM_MAX:
			CHECK_ERROR(dm_get_possible_range(mode, NULL, &new_range));
			break;

		default:
			return SCPI_ERROR_ILLEGAL_PARAMETER_VALUE;
		}

		switch (params->resolution.type) {
		case SCPI_NUM_NUMBER:
			if (params->resolution.value > new_range * WORST_RESOLUTION) {
				new_resolution = new_range * WORST_RESOLUTION;
			} else if (params->resolution.value < new_range * BEST_RESOLUTION * DECREASE_DELTA) {
				return SCPI_ERROR_CANNOT_ACHIEVE_REQUESTED_RESOLUTION;
			} else {
				new_resolution = params->resolution.value;
			}
			break;

		case SCPI_NUM_DEF:
		case SCPI_NUM_MIN:
			new_resolution = new_range * BEST_RESOLUTION;
			break;

		case SCPI_NUM_MAX:
			new_resolution = new_range * WORST_RESOLUTION;
			break;

		default:
			return SCPI_ERROR_ILLEGAL_PARAMETER_VALUE;
		} */

	}	//	if (params)


	return SCPI_ERROR_OK;
}

int16_t dm_get_possible_range(const scpimm_mode_t mode, double* const min_range, double* const max_range) {

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

int16_t dm_get_possible_resolution(const scpimm_mode_t /* mode */, const double range, double* const min_resolution, double* const max_resolution) {

	/* store function arguments for later analysis */
	dm_get_possible_resolution_last_args.mode = mode;
	dm_get_possible_resolution_last_args.range = range;
	if (min_resolution) {
		dm_get_possible_resolution_last_args.min_resolution = min_resolution;
		dm_get_possible_resolution_last_args.min_resolution_is_null = FALSE;
	} else {
		dm_get_possible_resolution_last_args.min_resolution_is_null = TRUE;
	}
	if (max_resolution) {
		dm_get_possible_resolution_last_args.max_resolution = max_resolution;
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
