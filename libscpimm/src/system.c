#include "system.h"
#include "utils.h"

static bool_t state = SCPIMM_DEFAULT_BEEPER_STATE;

scpi_result_t SCPIMM_system_beeper(scpi_t* context) {
	if (SCPIMM_INTERFACE(context)->beep) {
		SCPIMM_INTERFACE(context)->beep();
	}
    return SCPI_RES_OK;
}

scpi_result_t SCPIMM_system_beeper_state(scpi_t* context) {
	bool_t b;
    if (SCPI_ParamBool(context, &b, TRUE)) {
		state = b;
	}
    return SCPI_RES_OK;
}

scpi_result_t SCPIMM_system_beeper_stateQ(scpi_t* context) {
	SCPI_ResultBool(context, state);
    return SCPI_RES_OK;
}

