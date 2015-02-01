#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "sense.h"
#include "configure.h"
#include "utils.h"

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
	const scpimm_interface_t* const intf = ctx->interface;
	const char* res = NULL;
	scpimm_mode_t mode;
	const int16_t err = intf->get_mode(&mode, NULL, NULL);

	if (SCPI_ERROR_OK != err) {
	    SCPI_ErrorPush(context, err);
    	return SCPI_RES_ERR;
	}

	switch (mode) {
		case SCPIMM_MODE_DCV: 
			res = "VOLTage:DC";
			break;

		case SCPIMM_MODE_DCV_RATIO:
			res = "VOLTage:DC:RATio";
			break;

		case SCPIMM_MODE_ACV:
			res = "VOLTage:AC";
			break;

		case SCPIMM_MODE_DCC:
			res = "CURRent:DC";
			break;

		case SCPIMM_MODE_ACC:
			res = "CURRent:AC";
			break;

		case SCPIMM_MODE_RESISTANCE_2W:
			res = "RESistance";
			break;

		case SCPIMM_MODE_RESISTANCE_4W:
			res = "FRESistance";
			break;
	
		case SCPIMM_MODE_FREQUENCY:
			res = "FREQuency";
			break;

		case SCPIMM_MODE_PERIOD:
			res = "PERiod";
			break;

		case SCPIMM_MODE_CONTINUITY:
			res = "CONTinuity";
			break;

		case SCPIMM_MODE_DIODE:
			res = "DIODe";
			break;

		default:
			/* TODO: valid error code */
			SCPI_ErrorPush(context, SCPI_ERROR_SUFFIX_NOT_ALLOWED);
			return SCPI_RES_ERR;
    }

	SCPI_ResultText(context, res);
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_voltage_dc_range(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_voltage_dc_rangeQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_voltage_ac_range(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_voltage_ac_rangeQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_current_dc_range(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_current_dc_rangeQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_current_ac_range(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_current_ac_rangeQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_resistance_range(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_resistance_rangeQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_fresistance_range(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sense_fresistance_rangeQ(scpi_t* context) {
	(void) context;
	return SCPI_RES_OK;
}

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

