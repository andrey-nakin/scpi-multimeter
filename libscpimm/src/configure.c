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

		case SCPIMM_MODE_FREQUENCY:
			return SCPI_UNIT_HERTZ;

		case SCPIMM_MODE_PERIOD:
			return SCPI_UNIT_SECONDS;
	}
	return SCPI_UNIT_NONE;
}

static bool_t validate_number(scpi_t* context, scpimm_mode_t mode, const scpi_number_t* num) {
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

	if (!(SCPIMM_INTERFACE(context)->supported_modes() & mode)) {
		/* given mode is not supported */
	    SCPI_ErrorPush(context, SCPI_ERROR_UNDEFINED_HEADER);
    	return SCPI_RES_ERR;
	}

    if (SCPI_ParamNumber(context, &range, FALSE)) {
		if (!validate_number(context, mode, &range)) {
			return SCPI_RES_ERR;
		}
	} else {
		range.type = SCPI_NUM_DEF;
	}

    if (SCPI_ParamNumber(context, &resolution, FALSE)) {
		if (!validate_number(context, mode, &resolution)) {
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

static scpi_result_t configure_noarg_impl(scpi_t* context, scpimm_mode_t mode) {
	if (!(SCPIMM_INTERFACE(context)->supported_modes() & mode)) {
		/* given mode is not supported */
	    SCPI_ErrorPush(context, SCPI_ERROR_UNDEFINED_HEADER);
    	return SCPI_RES_ERR;
	}

	expectNoParams(context);

	if (context->cmd_error) {
    	return SCPI_RES_ERR;
	}

	return SCPIMM_do_configure(context, mode, NULL, NULL);
}

scpi_result_t SCPIMM_do_configure(scpi_t* context, scpimm_mode_t mode, const scpi_number_t* range, const scpi_number_t* resolution) {
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
	const scpimm_interface_t* const intf = ctx->interface;
	scpi_number_t *rangeVar = NULL, *resolutionVar = NULL;
	int16_t err;

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

	err = intf->set_mode(mode, range, resolution);
	if (SCPI_ERROR_OK != err) {
	    SCPI_ErrorPush(context, err);
    	return SCPI_RES_ERR;
	}

	if (range && rangeVar) {
		*rangeVar = *range;
	}
	if (resolution && resolutionVar) {
		*resolutionVar = *resolution;
	}

	return SCPIMM_measure_preset(context);
}

scpi_result_t SCPIMM_configureQ(scpi_t* context) {
	(void) context;	
	/* TODO */
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

