#include <scpimm/scpimm.h>
#include "input.h"
#include "scpimm_internal.h"
#include "utils.h"

scpi_result_t SCPIMM_do_set_input_impedance_auto(scpi_t* context, bool_t state) {
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
	const scpimm_interface_t* const intf = ctx->interface;

	if (intf->set_input_impedance_auto) {
		if (!intf->set_input_impedance_auto(state)) {
			signalInternalError(context);
			return SCPI_RES_ERR;
		}
	}

	ctx->input_impedance_auto_state = state;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_input_impedance_auto(scpi_t* context) {
	bool_t b;
    if (SCPI_ParamBool(context, &b, TRUE)) {
		return SCPIMM_do_set_input_impedance_auto(context, b);
	}
    return SCPI_RES_ERR;
}

scpi_result_t SCPIMM_input_impedance_autoQ(scpi_t* context) {
	SCPI_ResultBool(context, SCPIMM_CONTEXT(context)->input_impedance_auto_state);
    return SCPI_RES_OK;
}

