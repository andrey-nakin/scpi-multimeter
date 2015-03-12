#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "sense.h"
#include "configure.h"
#include "utils.h"
#include "dmm.h"

scpi_result_t SCPIMM_sense_function(scpi_t* context) {
    const char* param;
    size_t param_len;
	uint16_t mode;

    if (!SCPI_ParamString(context, &param, &param_len, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (matchCommand("VOLTage:DC", param, param_len)) {
		mode = SCPIMM_MODE_DCV;
    } else if (matchCommand("VOLTage:DC:RATio", param, param_len)) {
		mode = SCPIMM_MODE_DCV_RATIO;
    } else if (matchCommand("VOLTage:AC", param, param_len)) {
		mode = SCPIMM_MODE_ACV;
    } else if (matchCommand("CURRent:DC", param, param_len)) {
		mode = SCPIMM_MODE_DCC;
    } else if (matchCommand("CURRent:AC", param, param_len)) {
		mode = SCPIMM_MODE_ACC;
    } else if (matchCommand("RESistance", param, param_len)) {
		mode = SCPIMM_MODE_RESISTANCE_2W;
    } else if (matchCommand("FRESistance", param, param_len)) {
		mode = SCPIMM_MODE_RESISTANCE_4W;
    } else if (matchCommand("FREQuency", param, param_len)) {
		mode = SCPIMM_MODE_FREQUENCY;
    } else if (matchCommand("PERiod", param, param_len)) {
		mode = SCPIMM_MODE_PERIOD;
    } else if (matchCommand("CONTinuity", param, param_len)) {
		mode = SCPIMM_MODE_CONTINUITY;
    } else if (matchCommand("DIODe", param, param_len)) {
		mode = SCPIMM_MODE_DIODE;
	} else {
		/* TODO: valid error code */
		SCPI_ErrorPush(context, SCPI_ERROR_SUFFIX_NOT_ALLOWED);
		return SCPI_RES_ERR;
	}

	return SCPIMM_do_configure(context, mode, NULL, NULL);
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

static scpi_result_t SCPIMM_sense_range_impl(scpi_t* const context, const scpimm_mode_t mode) {
	// TODO
	(void) context;
	(void) mode;
	return SCPI_RES_OK;
}

static scpi_result_t SCPIMM_sense_rangeQ_impl(scpi_t* const context, const scpimm_mode_t mode) {
	// TODO
	(void) context;
	(void) mode;
	return SCPI_RES_OK;
}

#define DECL_SENSE_HANDLERS(mode, func) \
	scpi_result_t SCPIMM_sense_ ## func ## _range(scpi_t* context) {	\
		return SCPIMM_sense_range_impl(context, SCPIMM_MODE_ ## mode);	\
	}	\
	\
	scpi_result_t SCPIMM_sense_ ## func ## _rangeQ(scpi_t* context) {	\
		return SCPIMM_sense_rangeQ_impl(context, SCPIMM_MODE_ ## mode);	\
	}

DECL_SENSE_HANDLERS(DCV, voltage_dc)
DECL_SENSE_HANDLERS(ACV, voltage_ac)
DECL_SENSE_HANDLERS(DCC, current_dc)
DECL_SENSE_HANDLERS(ACC, current_ac)
DECL_SENSE_HANDLERS(RESISTANCE_2W, resistance)
DECL_SENSE_HANDLERS(RESISTANCE_4W, fresistance)

scpi_result_t SCPIMM_sense_frequency_voltage_range(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_frequency_voltage_rangeQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_period_voltage_range(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_period_voltage_rangeQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_voltage_dc_range_auto(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_voltage_dc_range_autoQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_voltage_ac_range_auto(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_voltage_ac_range_autoQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_current_dc_range_auto(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_current_dc_range_autoQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_current_ac_range_auto(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_current_ac_range_autoQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_resistance_range_auto(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_resistance_range_autoQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_fresistance_range_auto(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_fresistance_range_autoQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_frequency_voltage_range_auto(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_frequency_voltage_range_autoQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_period_voltage_range_auto(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_period_voltage_range_autoQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

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

scpi_result_t SCPIMM_sense_voltage_dc_nplcycles(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_voltage_dc_nplcyclesQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_current_dc_nplcycles(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_current_dc_nplcyclesQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_resistance_nplcycles(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_resistance_nplcyclesQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_fresistance_nplcycles(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_fresistance_nplcyclesQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_frequency_aperture(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_frequency_apertureQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_period_aperture(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_period_apertureQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_detector_bandwidth(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_detector_bandwidthQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_zero_auto(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_zero_autoQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

