#include <scpimm/scpimm.h>
#include "configure.h"
#include "utils.h"

static bool_t paramRange(scpi_t* context, float* value, bool_t mandatory) {
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
		num_len = strToDouble(param, &d);
		if (num_len != param_len) {
		    SCPI_ErrorPush(context, SCPI_ERROR_SUFFIX_NOT_ALLOWED);
		    return FALSE;
		}
		*value = (float) d;
    }

    return TRUE;
}

static bool_t paramResolution(scpi_t* context, float* value, bool_t mandatory) {
	return paramRange(context, value, mandatory);
}

static scpi_result_t configure_2arg_impl(scpi_t* context, int mode) {
    float range = SCPIMM_RANGE_DEF, resolution = SCPIMM_RESOLUTION_DEF;

    paramRange(context, &range, false);
    paramResolution(context, &resolution, false);

	if (context->cmd_error) {
    	return SCPI_RES_ERR;
	}

	return SCPIMM_do_configure(context, mode, range, resolution);
}

scpi_result_t SCPIMM_do_configure(scpi_t* context, int mode, float range, float resolution) {
	SCPIMM_stop_mesurement();
	SCPIMM_clear_return_buffer();

	// TODO
//	if (0 != SCPIMM_INTERFACE(context)->set_mode(mode, range, resolution)) {
//	    SCPI_ErrorPush(context, SCPI_ERROR_UNDEFINED_HEADER);
//    	return SCPI_RES_ERR;
//	}

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
	return configure_2arg_impl(context, SCPIMM_MODE_CONTINUITY);
}

scpi_result_t SCPIMM_configure_diode(scpi_t* context) {
	return configure_2arg_impl(context, SCPIMM_MODE_DIODE);
}

