#include <scpimm/scpimm.h>
#include "dmm.h"
#include "utils.h"
#include "input.h"

scpi_result_t SCPIMM_measure_preset(scpi_t* context) {
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);

	ctx->sample_count_num = 1;
	ctx->trigger_count_num = 1;
	ctx->infinite_trigger_count = false;
	ctx->trigger_src = SCPIMM_TRIG_IMM;

	/* TODO
AC Filter (DET:BAND)                20 Hz (medium filter)
Autozero (ZERO:AUTO)                OFF if resolution setting results in NPLC < 1;
                                    ON if resolution setting results in NPLC ≥ 1
                                    OFF (fixed at 10 MΩ for all dc voltage ranges)
Trigger Delay (TRIG:DEL)            Automatic delay
Math Function (CALCulate subsystem) OFF
	*/

	return SCPIMM_do_set_input_impedance_auto(context, FALSE);
}

scpi_result_t SCPIMM_initiate(scpi_t* context) {
	/* TODO */
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_readQ(scpi_t* context) {
	/* TODO */
	(void) context;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_fetchQ(scpi_t* context) {
	/* TODO */
	(void) context;
	return SCPI_RES_OK;
}

