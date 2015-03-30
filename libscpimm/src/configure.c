#include <string.h>
#include <scpimm/scpimm.h>
#include "configure.h"
#include "dmm.h"
#include "utils.h"

static scpi_unit_t detect_units(scpimm_mode_t mode) {
	switch (mode) {
		case SCPIMM_MODE_DCV:
		case SCPIMM_MODE_DCV_RATIO:
		case SCPIMM_MODE_ACV:
			return SCPI_UNIT_VOLT;

		case SCPIMM_MODE_DCC:
		case SCPIMM_MODE_ACC:
			return SCPI_UNIT_AMPER;

		case SCPIMM_MODE_RESISTANCE_2W:
		case SCPIMM_MODE_RESISTANCE_4W:
			return SCPI_UNIT_OHM;
	}
	return SCPI_UNIT_NONE;
}

static scpi_bool_t validate_range(scpi_t* context, scpimm_mode_t mode, const scpi_number_t* num) {
	if (	num->type != SCPI_NUM_MIN 
			&& num->type != SCPI_NUM_MAX
			&& num->type != SCPI_NUM_DEF
			&& num->type != SCPI_NUM_AUTO
			&& num->type != SCPI_NUM_NUMBER ) {
		/* invalid value */
		/* TODO: correct error number */
		SCPI_ErrorPush(context, SCPI_ERROR_UNDEFINED_HEADER);
		return FALSE;
	}
		
	if (	num->type == SCPI_NUM_NUMBER
			&& num->unit != SCPI_UNIT_NONE 
			&& num->unit != detect_units(mode)) {

		/* invalid units */
		/* TODO: correct error number */
		SCPI_ErrorPush(context, SCPI_ERROR_UNDEFINED_HEADER);
		return FALSE;
	}

	return TRUE;
}

static scpi_bool_t validate_resolution(scpi_t* context, scpimm_mode_t mode, const scpi_number_t* num) {
	if (	num->type != SCPI_NUM_MIN
			&& num->type != SCPI_NUM_MAX
			&& num->type != SCPI_NUM_DEF
			&& num->type != SCPI_NUM_NUMBER ) {
		/* invalid value */
		/* TODO: correct error number */
		SCPI_ErrorPush(context, SCPI_ERROR_UNDEFINED_HEADER);
		return FALSE;
	}

	if (	num->type == SCPI_NUM_NUMBER
			&& num->unit != SCPI_UNIT_NONE
			&& num->unit != detect_units(mode)) {

		/* invalid units */
		/* TODO: correct error number */
		SCPI_ErrorPush(context, SCPI_ERROR_UNDEFINED_HEADER);
		return FALSE;
	}

	return TRUE;
}

static scpi_result_t configure_2arg_impl(scpi_t* context, scpimm_mode_t mode) {
    scpi_number_t range, resolution;

    if (SCPI_ParamNumber(context, &range, FALSE)) {
		if (!validate_range(context, mode, &range)) {
			return SCPI_RES_ERR;
		}
	} else {
		range.type = SCPI_NUM_DEF;
	}

    if (SCPI_ParamNumber(context, &resolution, FALSE)) {
		if (!validate_resolution(context, mode, &resolution)) {
			return SCPI_RES_ERR;
		}
	} else {
		resolution.type = SCPI_NUM_DEF;
	}

	expectNoParams(context);

	if (context->cmd_error) {
    	return SCPI_RES_ERR;
	}

	return SCPIMM_do_configure(context, mode, &range, &resolution);
}

/* static scpi_result_t configure_noarg_impl(scpi_t* context, scpimm_mode_t mode) {
	expectNoParams(context);

	if (context->cmd_error) {
    	return SCPI_RES_ERR;
	}

	return SCPIMM_do_configure(context, mode, NULL, NULL);
} */

scpi_result_t SCPIMM_do_configure(scpi_t* context, scpimm_mode_t mode, const scpi_number_t* range, const scpi_number_t* resolution) {
	int16_t err;

	// TODO is all the stuff below actual?
	SCPIMM_stop_mesurement();
	SCPIMM_clear_return_buffer();
	SCPIMM_measure_preset(context);	/* TODO returning error code is not checked */

	err = SCPIMM_set_mode(context, mode, range, TRUE, NULL, resolution);
	if (SCPI_ERROR_OK != err) {
	    SCPI_ErrorPush(context, err);
    	return SCPI_RES_ERR;
	}

	return SCPI_RES_OK;
}

static int16_t configureQuery(scpi_t* context) {
	scpimm_mode_t mode;
	scpi_bool_t no_params;
	scpimm_mode_params_t params;
	int16_t err;
	const char* mode_name;
    char buf[48], *end;
    const double *ranges, *resolutions;
    size_t current_range;

	CHECK_SCPI_ERROR(SCPIMM_INTERFACE(context)->get_mode(&mode, &params));
	no_params = FALSE;	// SCPIMM_MODE_CONTINUITY == mode || SCPIMM_MODE_DIODE == mode;
	if (!no_params) {
		CHECK_SCPI_ERROR(SCPIMM_INTERFACE(context)->get_numeric_param(mode, SCPIMM_PARAM_RANGE, &current_range));
		CHECK_SCPI_ERROR(SCPIMM_INTERFACE(context)->get_numeric_param_values(mode, SCPIMM_PARAM_RANGE, &ranges));
		CHECK_SCPI_ERROR(SCPIMM_INTERFACE(context)->get_allowed_resolutions(mode, current_range, &resolutions));
	}

	mode_name = SCPIMM_mode_name(mode);
	if (NULL == mode_name) {
		return SCPI_ERROR_UNKNOWN;
	}

	strcpy(buf, mode_name);
	if (!no_params) {
		strcat(buf, " ");
		end = strchr(buf, '\0');
		end += double_to_str(end, ranges[current_range]);
		*end++ = ',';
		end += double_to_str(end, resolutions[params.resolution_index]);
		*end = '\0';
	}
	SCPI_ResultText(context, buf);

	return SCPI_ERROR_OK;
}

scpi_result_t SCPIMM_configureQ(scpi_t* context) {
	int16_t err = configureQuery(context);
	if (SCPI_ERROR_OK != err) {
	    SCPI_ErrorPush(context, err);
    	return SCPI_RES_ERR;
	}

	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_configure_voltage_dc(scpi_t* context) {
	return configure_2arg_impl(context, SCPIMM_MODE_DCV);
}

scpi_result_t SCPIMM_configure_voltage_dc_ratio(scpi_t* context) {
	return configure_2arg_impl(context, SCPIMM_MODE_DCV_RATIO);
}

scpi_result_t SCPIMM_configure_voltage_ac(scpi_t* context) {
	return configure_2arg_impl(context, SCPIMM_MODE_ACV);
}

scpi_result_t SCPIMM_configure_current_dc(scpi_t* context) {
	return configure_2arg_impl(context, SCPIMM_MODE_DCC);
}

scpi_result_t SCPIMM_configure_current_ac(scpi_t* context) {
	return configure_2arg_impl(context, SCPIMM_MODE_ACC);
}

scpi_result_t SCPIMM_configure_resistance(scpi_t* context) {
	return configure_2arg_impl(context, SCPIMM_MODE_RESISTANCE_2W);
}

scpi_result_t SCPIMM_configure_fresistance(scpi_t* context) {
	return configure_2arg_impl(context, SCPIMM_MODE_RESISTANCE_4W);
}
