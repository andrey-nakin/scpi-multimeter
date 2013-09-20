#include "system.h"
#include "utils.h"

scpi_result_t SCPIMM_system_beeper(scpi_t* context) {
	if (SCPIMM_INTERFACE(context)->beep) {
		SCPIMM_INTERFACE(context)->beep();
	}
    return SCPI_RES_OK;
}

scpi_result_t SCPIMM_system_beeper_state(scpi_t* context) {
	bool_t b;
    if (SCPI_ParamBool(context, &b, TRUE)) {
		SCPIMM_CONTEXT(context)->beeper_state = b;
	}
    return SCPI_RES_OK;
}

scpi_result_t SCPIMM_system_beeper_stateQ(scpi_t* context) {
	SCPI_ResultBool(context, SCPIMM_CONTEXT(context)->beeper_state);
    return SCPI_RES_OK;
}

scpi_result_t SCPIMM_system_local(scpi_t* context) {
	SCPIMM_set_remote(context, FALSE, FALSE);
    return SCPI_RES_OK;
}

scpi_result_t SCPIMM_system_remote(scpi_t* context) {
	SCPIMM_set_remote(context, TRUE, FALSE);
    return SCPI_RES_OK;
}

scpi_result_t SCPIMM_system_rwlock(scpi_t* context) {
	SCPIMM_set_remote(context, TRUE, TRUE);
    return SCPI_RES_OK;
}

void SCPIMM_set_remote(scpi_t* context, bool_t remote, bool_t lock) {
	if (SCPIMM_INTERFACE(context)->remote) {
		SCPIMM_INTERFACE(context)->remote(remote, lock);
	}
}

