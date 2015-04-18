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
#include <scpimm/scpimm.h>
#include "configure.h"
#include "dmm.h"
#include "utils.h"

static scpi_unit_t detect_units(scpimm_mode_t mode) {
	switch (mode) {
		case SCPIMM_MODE_DCV:
		case SCPIMM_MODE_DCV_RATIO:
		case SCPIMM_MODE_ACV:
		case SCPIMM_MODE_ACV_RATIO:
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
		SCPI_ErrorPush(context, SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE);
		return FALSE;
	}
		
	if (	num->type == SCPI_NUM_NUMBER
			&& num->unit != SCPI_UNIT_NONE 
			&& num->unit != detect_units(mode)) {

		/* invalid units */
		SCPI_ErrorPush(context, SCPIMM_ERROR_INVALID_SUFFIX);
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
		SCPI_ErrorPush(context, SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE);
		return FALSE;
	}

	if (	num->type == SCPI_NUM_NUMBER
			&& num->unit != SCPI_UNIT_NONE
			&& num->unit != detect_units(mode)) {

		/* invalid units */
		SCPI_ErrorPush(context, SCPIMM_ERROR_INVALID_SUFFIX);
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

	SCPIMM_stop_mesurement(SCPIMM_CONTEXT(context));
	SCPIMM_clear_return_buffer(context);
	if (SCPI_RES_OK != SCPIMM_measure_preset(context)) {
		return SCPI_RES_ERR;
	}

	err = SCPIMM_set_mode(context, mode, range, TRUE, NULL, resolution);
	if (SCPIMM_ERROR_OK != err) {
	    SCPI_ErrorPush(context, err);
    	return SCPI_RES_ERR;
	}

	return SCPI_RES_OK;
}

static int16_t configureQuery(scpi_t* context) {
	scpimm_mode_t mode;
	const scpi_bool_t no_params = FALSE; // SCPIMM_MODE_CONTINUITY == mode || SCPIMM_MODE_DIODE == mode;
	int16_t err;
	const char* mode_name;
    char buf[48], *end;
    const double *ranges, *resolutions;
    size_t current_range, current_resolution;

	CHECK_SCPIMM_ERROR(SCPIMM_INTERFACE(context)->get_mode(&mode));
	if (!no_params) {
		CHECK_SCPIMM_ERROR(SCPIMM_INTERFACE(context)->get_numeric_param(mode, SCPIMM_PARAM_RANGE, &current_range));
		CHECK_SCPIMM_ERROR(SCPIMM_INTERFACE(context)->get_numeric_param_values(mode, SCPIMM_PARAM_RANGE, &ranges));
		CHECK_SCPIMM_ERROR(SCPIMM_INTERFACE(context)->get_numeric_param(mode, SCPIMM_PARAM_RESOLUTION, &current_resolution));
		CHECK_SCPIMM_ERROR(SCPIMM_INTERFACE(context)->get_allowed_resolutions(mode, current_range, &resolutions));
	}

	mode_name = SCPIMM_mode_name(mode);
	if (NULL == mode_name) {
		return SCPIMM_ERROR_INVALID_SUFFIX;
	}

	strcpy(buf, mode_name);
	if (!no_params) {
		strcat(buf, " ");
		end = strchr(buf, '\0');
		end += double_to_str(end, ranges[current_range]);
		*end++ = ',';
		end += double_to_str(end, resolutions[current_resolution]);
		*end = '\0';
	}
	SCPI_ResultText(context, buf);

	return SCPIMM_ERROR_OK;
}

scpi_result_t SCPIMM_configureQ(scpi_t* context) {
	int16_t err = configureQuery(context);
	if (SCPIMM_ERROR_OK != err) {
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

scpi_result_t SCPIMM_configure_voltage_ac_ratio(scpi_t* context) {
	return configure_2arg_impl(context, SCPIMM_MODE_ACV_RATIO);
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
