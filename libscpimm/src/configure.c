#include <string.h>
#include <scpimm/scpimm.h>
#include "configure.h"
#include "utils.h"

static bool_t paramRange(scpi_t* context, float* value, bool_t mandatory, const char* units) {
    const char* param;
    size_t param_len;
    size_t num_len;
	double d;

    if (!value) {
        return FALSE;
    }

    if (!SCPI_ParamString(context, &param, &param_len, mandatory)) {
        return FALSE;
    }

    if (matchPattern("MIN", 3, param, param_len)) {
        *value = SCPIMM_RANGE_MIN;
    } else if (matchPattern("MAX", 3, param, param_len)) {
        *value = SCPIMM_RANGE_MAX;
    } else if (matchPattern("DEF", 3, param, param_len)) {
        *value = SCPIMM_RANGE_DEF;
    } else {
		bool_t gain_specified = FALSE;
		double gain = 1.0;
		const char* const lastptr = param + param_len;
		char* endptr;

		d = strtod(param, &endptr);
		while (endptr < lastptr && isspace(*endptr)) {
			++endptr;
		}
		while (endptr < lastptr) {
			if (!gain_specified && ('k' == *endptr || 'K' == *endptr)) {
				++endptr;
				gain = 1.0e3;
				gain_specified = TRUE;
			} else if (!gain_specified && ('m' == *endptr || 'M' == *endptr)) {
				++endptr;
				gain = 1.0e-3;
				gain_specified = TRUE;
			} else if (!gain_specified && ('u' == *endptr || 'U' == *endptr)) {
				++endptr;
				gain = 1.0e-6;
				gain_specified = TRUE;
			} else if (matchPattern(units, strlen(units), endptr, (lastptr - endptr))) {
				break;
			} else {
				SCPI_ErrorPush(context, SCPI_ERROR_SUFFIX_NOT_ALLOWED);
				return FALSE;
			}
		}
		*value = (float) (gain * d);
    }

    return TRUE;
}

static bool_t paramResolution(scpi_t* context, float* value, bool_t mandatory, const char* units) {
	return paramRange(context, value, mandatory, units);
}

static const char* make_units(uint16_t mode) {
	/* TODO check unit names */
	switch (mode) {
		case SCPIMM_MODE_DCV:
		case SCPIMM_MODE_DCV_RATIO:
		case SCPIMM_MODE_ACV:
			return "Volt";
		case SCPIMM_MODE_DCC:
		case SCPIMM_MODE_ACC:
			return "Amper";
		case SCPIMM_MODE_RESISTANCE_2W:
		case SCPIMM_MODE_RESISTANCE_4W:
			return "Ohm";
		case SCPIMM_MODE_FREQUENCY:
			return "Hz";
		case SCPIMM_MODE_PERIOD:
			return "S";
	}
	return NULL;
}

static scpi_result_t configure_2arg_impl(scpi_t* context, uint16_t mode) {
    float range = SCPIMM_RANGE_UNSPECIFIED, resolution = SCPIMM_RESOLUTION_UNSPECIFIED;
	const char* units = make_units(mode);

	if (!(SCPIMM_INTERFACE(context)->supported_modes() & mode)) {
		/* given mode is not supported */
	    SCPI_ErrorPush(context, SCPI_ERROR_UNDEFINED_HEADER);
    	return SCPI_RES_ERR;
	}

    paramRange(context, &range, FALSE, units);
    paramResolution(context, &resolution, FALSE, units);
	expectNoParams(context);

	if (context->cmd_error) {
    	return SCPI_RES_ERR;
	}

	return SCPIMM_do_configure(context, mode, range, resolution);
}

static scpi_result_t configure_noarg_impl(scpi_t* context, uint16_t mode) {
	if (!(SCPIMM_INTERFACE(context)->supported_modes() & mode)) {
		/* given mode is not supported */
	    SCPI_ErrorPush(context, SCPI_ERROR_UNDEFINED_HEADER);
    	return SCPI_RES_ERR;
	}

	expectNoParams(context);

	if (context->cmd_error) {
    	return SCPI_RES_ERR;
	}

	return SCPIMM_do_configure(context, mode, SCPIMM_RANGE_UNSPECIFIED, SCPIMM_RESOLUTION_UNSPECIFIED);
}

scpi_result_t SCPIMM_do_configure(scpi_t* context, uint16_t mode, float range, float resolution) {
	scpimm_context_t* ctx = SCPIMM_CONTEXT(context);
	float *rangeVar = NULL, *resolutionVar = NULL;

	SCPIMM_stop_mesurement();
	SCPIMM_clear_return_buffer();

	switch (mode) {
		case SCPIMM_MODE_DCV:
			rangeVar = &ctx->dcv_range;
			resolutionVar = &ctx->dcv_resolution;
			break;

		case SCPIMM_MODE_DCV_RATIO:
			rangeVar = &ctx->dcv_ratio_range;
			resolutionVar = &ctx->dcv_ratio_resolution;
			break;

		case SCPIMM_MODE_ACV:
			rangeVar = &ctx->acv_range;
			resolutionVar = &ctx->acv_resolution;
			break;

		case SCPIMM_MODE_DCC:
			rangeVar = &ctx->dcc_range;
			resolutionVar = &ctx->dcc_resolution;
			break;

		case SCPIMM_MODE_ACC:
			rangeVar = &ctx->acc_range;
			resolutionVar = &ctx->acc_resolution;
			break;

		case SCPIMM_MODE_RESISTANCE_2W:
			rangeVar = &ctx->resistance_range;
			resolutionVar = &ctx->resistance_resolution;
			break;

		case SCPIMM_MODE_RESISTANCE_4W:
			rangeVar = &ctx->fresistance_range;
			resolutionVar = &ctx->fresistance_resolution;
			break;

		case SCPIMM_MODE_FREQUENCY:
			rangeVar = &ctx->frequency_range;
			resolutionVar = &ctx->frequency_resolution;
			break;

		case SCPIMM_MODE_PERIOD:
			rangeVar = &ctx->period_range;
			resolutionVar = &ctx->period_resolution;
			break;
	}

	if (SCPIMM_RANGE_UNSPECIFIED == range && rangeVar) {
		range = *rangeVar;
	}
	if (SCPIMM_RESOLUTION_UNSPECIFIED == resolution && resolutionVar) {
		resolution = *resolutionVar;
	}

	if (!SCPIMM_INTERFACE(context)->set_mode(mode, range, resolution)) {
	    SCPI_ErrorPush(context, SCPI_ERROR_UNDEFINED_HEADER);	/* TODO error code ? */
    	return SCPI_RES_ERR;
	}

	ctx->mode = mode;
	if (rangeVar) {
		*rangeVar = range;
	}
	if (resolutionVar) {
		*resolutionVar = resolution;
	}

	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_configureQ(scpi_t* context) {
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

scpi_result_t SCPIMM_configure_frequency(scpi_t* context) {
	return configure_2arg_impl(context, SCPIMM_MODE_FREQUENCY);
}

scpi_result_t SCPIMM_configure_period(scpi_t* context) {
	return configure_2arg_impl(context, SCPIMM_MODE_PERIOD);
}

scpi_result_t SCPIMM_configure_continuity(scpi_t* context) {
	return configure_noarg_impl(context, SCPIMM_MODE_CONTINUITY);
}

scpi_result_t SCPIMM_configure_diode(scpi_t* context) {
	return configure_noarg_impl(context, SCPIMM_MODE_DIODE);
}

