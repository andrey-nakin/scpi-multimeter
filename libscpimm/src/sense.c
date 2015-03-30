#include <stdint.h>
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "sense.h"
#include "utils.h"
#include "dmm.h"

scpi_result_t SCPIMM_sense_function(scpi_t* context) {
	int16_t err;
    const char* param;
    size_t param_len;
    scpimm_mode_t mode;

    if (!SCPI_ParamString(context, &param, &param_len, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (matchCommand("VOLTage", param, param_len)) {
		mode = SCPIMM_MODE_DCV;
    } else if (matchCommand("VOLTage:DC", param, param_len)) {
		mode = SCPIMM_MODE_DCV;
    } else if (matchCommand("VOLTage:DC:RATio", param, param_len)) {
		mode = SCPIMM_MODE_DCV_RATIO;
    } else if (matchCommand("VOLTage:AC", param, param_len)) {
		mode = SCPIMM_MODE_ACV;
    } else if (matchCommand("CURRent", param, param_len)) {
		mode = SCPIMM_MODE_DCC;
    } else if (matchCommand("CURRent:DC", param, param_len)) {
		mode = SCPIMM_MODE_DCC;
    } else if (matchCommand("CURRent:AC", param, param_len)) {
		mode = SCPIMM_MODE_ACC;
    } else if (matchCommand("RESistance", param, param_len)) {
		mode = SCPIMM_MODE_RESISTANCE_2W;
    } else if (matchCommand("FRESistance", param, param_len)) {
		mode = SCPIMM_MODE_RESISTANCE_4W;
	} else {
		/* TODO: valid error code */
		SCPI_ErrorPush(context, SCPI_ERROR_SUFFIX_NOT_ALLOWED);
		return SCPI_RES_ERR;
	}

    CHECK_AND_PUSH_ERROR(SCPIMM_INTERFACE(context)->set_mode(mode, NULL));

	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_functionQ(scpi_t* context) {
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
	const char* res = NULL;
	scpimm_mode_t mode;
	const int16_t err = ctx->interface->get_mode(&mode, NULL);

	if (SCPI_ERROR_OK != err) {
	    SCPI_ErrorPush(context, err);
    	return SCPI_RES_ERR;
	}
	
	res = SCPIMM_mode_name(mode);
	if (NULL == res) {
		/* TODO: valid error code */
		SCPI_ErrorPush(context, SCPI_ERROR_SUFFIX_NOT_ALLOWED);
		return SCPI_RES_ERR;
    }

	SCPI_ResultText(context, res);
	return SCPI_RES_OK;
}

static scpi_result_t set_numeric_param(scpi_t* const context, const scpimm_mode_t mode, const scpimm_numeric_param_t param, scpi_bool_t min_max_allowed) {
	int16_t err;
    scpi_number_t value;
	const double* values;
    size_t value_index = SIZE_MAX;

    CHECK_AND_PUSH_ERROR(SCPIMM_INTERFACE(context)->get_numeric_param_values(mode, param, &values));

	if (!SCPI_ParamNumber(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}

	switch (value.type) {
	case SCPI_NUM_MIN:
		if (min_max_allowed) {
			value_index = min_value_index(values);
			break;
		}

	case SCPI_NUM_MAX:
		if (min_max_allowed) {
			value_index = max_value_index(values);
			break;
		}

	case SCPI_NUM_NUMBER:
		value_index = greater_or_equal_index(values, value.value);
		break;

	default:
		CHECK_AND_PUSH_ERROR(SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

    EXPECT_NO_PARAMS(context);
	CHECK_AND_PUSH_ERROR(SCPIMM_INTERFACE(context)->set_numeric_param(mode, param, value_index));

	return SCPI_RES_OK;
}

static scpi_result_t query_numeric_param(scpi_t* const context, const scpimm_mode_t mode, const scpimm_numeric_param_t param, scpi_bool_t min_max_allowed) {
	int16_t err;
	const double* values;
    size_t value_index = SIZE_MAX;

    CHECK_AND_PUSH_ERROR(SCPIMM_INTERFACE(context)->get_numeric_param_values(mode, param, &values));

    if (min_max_allowed) {
        scpi_number_t what;

		if (SCPI_ParamNumber(context, &what, FALSE)) {
			switch (what.type) {
			case SCPI_NUM_MIN:
				value_index = min_value_index(values);
				break;

			case SCPI_NUM_MAX:
				value_index = max_value_index(values);
				break;

			default:
				CHECK_AND_PUSH_ERROR(SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
			}
		}
    }

    EXPECT_NO_PARAMS(context);

    if (SIZE_MAX == value_index) {
    	CHECK_AND_PUSH_ERROR(SCPIMM_INTERFACE(context)->get_numeric_param(mode, param, &value_index));
    }

    SCPIMM_ResultDouble(context, values[value_index]);

    return SCPI_RES_OK;
}

static scpi_result_t set_bool_param(scpi_t* const context, const scpimm_mode_t mode, const scpimm_bool_param_t param) {
	int16_t err;
    scpi_bool_t value;

	if (!SCPI_ParamBool(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}
    EXPECT_NO_PARAMS(context);

	CHECK_AND_PUSH_ERROR(SCPIMM_INTERFACE(context)->set_bool_param(mode, param, value));

	return SCPI_RES_OK;
}

static scpi_result_t query_bool_param(scpi_t* const context, const scpimm_mode_t mode, const scpimm_bool_param_t param) {
	int16_t err;
    scpi_bool_t value;

    EXPECT_NO_PARAMS(context);

	CHECK_AND_PUSH_ERROR(SCPIMM_INTERFACE(context)->get_bool_param(mode, param, &value));
    SCPI_ResultBool(context, value);

    return SCPI_RES_OK;
}

static scpi_result_t set_global_bool_param(scpi_t* const context, const scpimm_bool_param_t param) {
	int16_t err;
    scpi_bool_t value;

	if (!SCPI_ParamBool(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}
    EXPECT_NO_PARAMS(context);

	CHECK_AND_PUSH_ERROR(SCPIMM_INTERFACE(context)->set_global_bool_param(param, value));

	return SCPI_RES_OK;
}

static scpi_result_t query_global_bool_param(scpi_t* const context, const scpimm_bool_param_t param) {
	int16_t err;
    scpi_bool_t value;

    EXPECT_NO_PARAMS(context);

	CHECK_AND_PUSH_ERROR(SCPIMM_INTERFACE(context)->get_global_bool_param(param, &value));
    SCPI_ResultBool(context, value);

    return SCPI_RES_OK;
}

#define DECL_SENSE_HANDLERS(mode, func) \
	scpi_result_t SCPIMM_sense_ ## func ## _range(scpi_t* context) {	\
		return set_numeric_param(context, SCPIMM_MODE_ ## mode, SCPIMM_PARAM_RANGE, TRUE);	\
	}	\
	\
	scpi_result_t SCPIMM_sense_ ## func ## _rangeQ(scpi_t* context) {	\
		return query_numeric_param(context, SCPIMM_MODE_ ## mode, SCPIMM_PARAM_RANGE, TRUE);	\
	}	\
	\
	scpi_result_t SCPIMM_sense_ ## func ## _range_auto(scpi_t* context) {	\
		return set_bool_param(context, SCPIMM_MODE_ ## mode, SCPIMM_PARAM_RANGE_AUTO);	\
	}	\
	\
	scpi_result_t SCPIMM_sense_ ## func ## _range_autoQ(scpi_t* context) {	\
		return query_bool_param(context, SCPIMM_MODE_ ## mode, SCPIMM_PARAM_RANGE_AUTO);	\
	}

#define DECL_SENSE_DC_HANDLERS(mode, func) \
	DECL_SENSE_HANDLERS(mode, func)	\
	\
	scpi_result_t SCPIMM_sense_ ## func ## _nplcycles(scpi_t* context) {	\
		return set_numeric_param(context, SCPIMM_MODE_ ## mode, SCPIMM_PARAM_NPLC, TRUE);	\
	}	\
	\
	scpi_result_t SCPIMM_sense_ ## func ## _nplcyclesQ(scpi_t* context) {	\
		return query_numeric_param(context, SCPIMM_MODE_ ## mode, SCPIMM_PARAM_NPLC, TRUE);	\
	}

DECL_SENSE_DC_HANDLERS(DCV, voltage_dc)
DECL_SENSE_HANDLERS(ACV, voltage_ac)
DECL_SENSE_DC_HANDLERS(DCC, current_dc)
DECL_SENSE_HANDLERS(ACC, current_ac)
DECL_SENSE_DC_HANDLERS(RESISTANCE_2W, resistance)
DECL_SENSE_DC_HANDLERS(RESISTANCE_4W, fresistance)

scpi_result_t SCPIMM_sense_voltage_dc_resolution(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_voltage_dc_resolutionQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_voltage_ac_resolution(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_voltage_ac_resolutionQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_current_dc_resolution(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_current_dc_resolutionQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_current_ac_resolution(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_current_ac_resolutionQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_resistance_resolution(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_resistance_resolutionQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_fresistance_resolution(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_fresistance_resolutionQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_zero_auto(scpi_t* context) {
	return set_global_bool_param(context, SCPIMM_PARAM_ZERO_AUTO);
}

scpi_result_t SCPIMM_sense_zero_autoQ(scpi_t* context) {
	return query_global_bool_param(context, SCPIMM_PARAM_ZERO_AUTO);
}
