#include "route.h"
#include "utils.h"

scpi_result_t SCPIMM_route_terminals(scpi_t* context) {
	/* default terminal */
	scpimm_terminal_state_t term = SCPIMM_TERM_FRONT;

	const scpimm_interface_t* const intf = SCPIMM_INTERFACE(context);
	if (intf->get_input_terminal) {
		/* query actual selected terminal */
		if (!intf->get_input_terminal(&term)) {
			signalInternalError(context);
			return SCPI_RES_ERR;
		}
	}

	SCPI_ResultString(context, term == SCPIMM_TERM_FRONT ? "FRON" : "REAR");
	return SCPI_RES_OK;
}

