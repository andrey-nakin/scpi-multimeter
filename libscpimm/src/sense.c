#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "sense.h"
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
		return FALSE;
	}

	return SCPIMM_do_configure(context, mode, 
		SCPIMM_RANGE_UNSPECIFIED, SCPIMM_RESOLUTION_UNSPECIFIED);
}

scpi_result_t SCPIMM_sense_functionQ(scpi_t* context) {
	const char* res = NULL;

	switch (SCPIMM_CONTEXT(context)->mode) {
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


