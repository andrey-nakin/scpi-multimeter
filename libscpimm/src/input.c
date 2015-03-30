#include <scpimm/scpimm.h>
#include "input.h"
#include "scpimm_internal.h"
#include "utils.h"

scpi_result_t SCPIMM_do_set_input_impedance_auto(scpi_t* const context, const scpi_bool_t state) {
	int16_t err;
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
	const scpimm_interface_t* const intf = ctx->interface;

	if (intf->set_global_bool_param) {
		CHECK_AND_PUSH_ERROR(intf->set_global_bool_param(SCPIMM_PARAM_INPUT_IMPEDANCE_AUTO, state));
	}

	ctx->input_impedance_auto_state = state;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_input_impedance_auto(scpi_t* const context) {
	scpi_bool_t b;
    if (SCPI_ParamBool(context, &b, TRUE)) {
		return SCPIMM_do_set_input_impedance_auto(context, b);
	}
    return SCPI_RES_ERR;
}

scpi_result_t SCPIMM_input_impedance_autoQ(scpi_t* const context) {
	SCPI_ResultBool(context, SCPIMM_CONTEXT(context)->input_impedance_auto_state);
    return SCPI_RES_OK;
}

