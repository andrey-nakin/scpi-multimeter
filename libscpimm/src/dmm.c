#include <scpimm/scpimm.h>
#include "dmm.h"
#include "utils.h"
#include "input.h"

#define SCPIMM_OVERFLOW 9.90000000E+37

static scpi_result_t initiate(scpi_t* context, scpimm_dst_t dst) {
	scpimm_context_t* ctx = SCPIMM_CONTEXT(context);

	if (SCPIMM_STATE_IDLE != SCPIMM_get_state(context)) {
	    SCPI_ErrorPush(context, SCPI_ERROR_INIT_IGNORED);
		return SCPI_RES_ERR;
	}

	ctx->dst = dst;
	ctx->sample_count = ctx->sample_count_num;
	ctx->trigger_count = ctx->trigger_count_num;

	return SCPIMM_set_state(context, SCPIMM_STATE_WAIT_FOR_TRIGGER);
}

static scpi_result_t wait_for_idle(scpi_t* context) {
	while (SCPIMM_STATE_IDLE != SCPIMM_get_state(context)) {
		/* TODO sleep */
	}

	return SCPI_RES_OK;
}

static scpi_result_t start_measure(scpi_t* context) {
	scpi_result_t result;

	if (SCPI_RES_OK == (result = SCPIMM_set_state(context, SCPIMM_STATE_MEASURE))) {
		const scpimm_interface_t* intf = SCPIMM_INTERFACE(context);
		if (!intf->start_measure || !intf->start_measure()) {
			signalInternalError(context);
			SCPIMM_set_state(context, SCPIMM_STATE_IDLE);
			result = SCPI_RES_ERR;
		}
	}

	return result;
}

scpi_result_t SCPIMM_measure_preset(scpi_t* context) {
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
	scpi_result_t result;

	ctx->state = SCPIMM_STATE_IDLE;
	ctx->sample_count_num = 1;
	ctx->trigger_count_num = 1;
	ctx->infinite_trigger_count = false;
	ctx->trigger_src = SCPIMM_TRIG_IMM;
	ctx->buf_tail = 0;
	ctx->buf_head = 0;

	/* TODO
AC Filter (DET:BAND)                20 Hz (medium filter)
Autozero (ZERO:AUTO)                OFF if resolution setting results in NPLC < 1;
                                    ON if resolution setting results in NPLC ≥ 1
                                    OFF (fixed at 10 MΩ for all dc voltage ranges)
Trigger Delay (TRIG:DEL)            Automatic delay
Math Function (CALCulate subsystem) OFF
	*/

	result = SCPIMM_do_set_input_impedance_auto(context, FALSE);
	if (SCPI_RES_OK == result) {
		result = SCPIMM_set_state(context, SCPIMM_STATE_IDLE);
	}
	return result;
}

scpi_result_t SCPIMM_set_state(scpi_t* context, scpimm_state_t new_state) {
	scpi_result_t result = SCPI_RES_OK;
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);

	if (ctx->state != new_state) {
		ctx->state = new_state;

		switch (new_state) {
			case SCPIMM_STATE_WAIT_FOR_TRIGGER:
				if (SCPIMM_TRIG_IMM == ctx->trigger_src) {
					/* immediate trigger */
					result = start_measure(context);
				}
				break;

			default:
				break;
		}
	}

	return result;
}

scpimm_state_t SCPIMM_get_state(scpi_t* context) {
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
	const scpimm_state_t state = ctx->state;
	return state;
}

void SCPIMM_read_value(const scpi_number_t* value) {
	scpi_t* const context = SCPI_context();
	volatile scpimm_context_t* const ctx = SCPIMM_context();
	if (SCPIMM_STATE_MEASURE != ctx->state) {
		/* measurement is not expected now */
		return;
	}

	const double v = value->type != SCPI_NUM_NUMBER ? SCPIMM_OVERFLOW : value->value;

	switch (ctx->dst) {
		case SCPIMM_DST_OUT:
			/* send value directly to output buffer */
			SCPI_ResultDouble(context, v);
			break;

		case SCPIMM_DST_BUF:
			/* put value directly to reading buffer */
			if (ctx->buf_tail %= SCPIMM_BUF_LEN != ctx->buf_tail) {
				ctx->buf[ctx->buf_tail++] = v;
				ctx->buf_tail %= SCPIMM_BUF_LEN;
			}
			break;
	}

	if (0 == --ctx->sample_count) {
		if (0 == --ctx->trigger_count) {
			SCPIMM_set_state(context, SCPIMM_STATE_IDLE);
		} else {
			SCPIMM_set_state(context, SCPIMM_STATE_WAIT_FOR_TRIGGER);
		}
	}
}

scpi_result_t SCPIMM_initiate(scpi_t* context) {
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);

	if (ctx->sample_count_num * ctx->trigger_count_num > SCPIMM_BUF_CAPACITY) {
	    SCPI_ErrorPush(context, SCPI_ERROR_INSUFFICIENT_MEMORY);
		return SCPI_RES_ERR;
	}

	return initiate(context, SCPIMM_DST_BUF);
}

scpi_result_t SCPIMM_readQ(scpi_t* context) {
	scpi_result_t result;
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);

	/* cannot initiate bus trigger by READ? command */
	if (SCPIMM_TRIG_BUS == ctx->trigger_src) {
	    SCPI_ErrorPush(context, SCPI_ERROR_TRIGGER_DEADLOCK);
		return SCPI_RES_ERR;
	}

	if (SCPI_RES_OK != (result = initiate(context, SCPIMM_DST_OUT))) {
		return result;
	}

	/* wait for measurement complete */
	if (SCPI_RES_OK != (result = wait_for_idle(context))) {
		return result;
	}

	return result;
}

scpi_result_t SCPIMM_fetchQ(scpi_t* context) {
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
	scpi_result_t result;

	/* wait for measurement complete */
	if (SCPI_RES_OK != (result = wait_for_idle(context))) {
		return result;
	}

	if (ctx->buf_tail == ctx->buf_head) {
		/* no data to transfer */
	    SCPI_ErrorPush(context, SCPI_ERROR_DATA_STALE);
		return SCPI_RES_ERR;
	}

	SCPI_ResultDouble(context, ctx->buf[ctx->buf_head++]);
	ctx->buf_head %= SCPIMM_BUF_LEN;
	return result;
}

scpi_result_t SCPIMM_trg(scpi_t* context) {
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
	
	if (	SCPIMM_TRIG_BUS != ctx->trigger_src		
			|| SCPIMM_STATE_WAIT_FOR_TRIGGER != SCPIMM_get_state(context)) {
	    SCPI_ErrorPush(context, SCPI_ERROR_TRIGGER_IGNORED);
		return SCPI_RES_ERR;
	}

	return start_measure(context);
}

