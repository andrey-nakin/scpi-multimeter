#include "system.h"
#include "scpimm_internal.h"
#include "utils.h"

scpi_result_t SCPIMM_system_beeper(scpi_t* context) {
	int16_t err;
	if (SCPIMM_INTERFACE(context)->beep) {
		CHECK_AND_PUSH_ERROR(SCPIMM_INTERFACE(context)->beep());
	}
    return SCPI_RES_OK;
}

scpi_result_t SCPIMM_system_beeper_state(scpi_t* context) {
	scpi_bool_t b;
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
	int16_t err;
	CHECK_AND_PUSH_ERROR(SCPIMM_set_remote(context, FALSE, FALSE));
    return SCPI_RES_OK;
}

scpi_result_t SCPIMM_system_remote(scpi_t* context) {
	int16_t err;
	CHECK_AND_PUSH_ERROR(SCPIMM_set_remote(context, TRUE, FALSE));
    return SCPI_RES_OK;
}

scpi_result_t SCPIMM_system_rwlock(scpi_t* context) {
	int16_t err;
	CHECK_AND_PUSH_ERROR(SCPIMM_set_remote(context, TRUE, TRUE));
    return SCPI_RES_OK;
}

int16_t SCPIMM_set_remote(scpi_t* const context, const scpi_bool_t remote, const scpi_bool_t lock) {
	int16_t err;
	scpimm_interface_t* const intf = SCPIMM_INTERFACE(context);

	CHECK_SCPI_ERROR(intf->set_global_bool_param(SCPIMM_PARAM_REMOTE, remote));
	CHECK_SCPI_ERROR(intf->set_global_bool_param(SCPIMM_PARAM_LOCK, lock));

	return SCPI_ERROR_OK;
}
