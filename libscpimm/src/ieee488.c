#include "ieee488.h"
#include "scpimm_internal.h"
#include "utils.h"

scpi_result_t SCPIMM_IdnQ(scpi_t* const context) {
	const scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
	const scpimm_interface_t* const intf = ctx->interface;

    SCPI_ResultString(context, intf->get_idn ? intf->get_idn() : "HEWLETT-PACKARD,34401A,0,11-5-2");
    return SCPI_RES_OK;
}

