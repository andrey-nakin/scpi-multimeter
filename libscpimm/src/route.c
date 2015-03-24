#include "route.h"
#include "scpimm_internal.h"
#include "utils.h"

scpi_result_t SCPIMM_route_terminals(scpi_t* const context) {
	int16_t err;

	/* default terminal */
	scpimm_terminal_state_t term = SCPIMM_TERM_FRONT;

	const scpimm_interface_t* const intf = SCPIMM_INTERFACE(context);
	if (intf->get_input_terminal) {
		/* query actual selected terminal */
		CHECK_AND_PUSH_ERROR(intf->get_input_terminal(&term));
	}

	SCPI_ResultString(context, term == SCPIMM_TERM_FRONT ? "FRON" : "REAR");
	return SCPI_RES_OK;
}

