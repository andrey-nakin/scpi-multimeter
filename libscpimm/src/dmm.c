#include <scpimm/scpimm.h>
#include "dmm.h"
#include "utils.h"
#include "input.h"

#define SCPIMM_OVERFLOW 9.90000000E+37
#define	DELTA 1.0e-6
#define INCREASE_DELTA (1 + DELTA)
#define DECREASE_DELTA (1 - DELTA)

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
	ctx->trigger_delay = 0;
	ctx->trigger_auto_delay = TRUE;
	ctx->infinite_trigger_count = FALSE;
	ctx->trigger_src = SCPIMM_TRIG_IMM;
	ctx->buf_tail = 0;
	ctx->buf_head = 0;

	/* TODO
AC Filter (DET:BAND)                20 Hz (medium filter)
Autozero (ZERO:AUTO)                OFF if resolution setting results in NPLC < 1;
                                    ON if resolution setting results in NPLC ≥ 1
                                    OFF (fixed at 10 MΩ for all dc voltage ranges)
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
		if (!ctx->infinite_trigger_count && 0 == --ctx->trigger_count) {
			SCPIMM_set_state(context, SCPIMM_STATE_IDLE);
		} else {
			SCPIMM_set_state(context, SCPIMM_STATE_WAIT_FOR_TRIGGER);
		}
	}
}

scpi_result_t SCPIMM_initiate(scpi_t* context) {
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);

	if (	ctx->infinite_trigger_count
			|| ctx->sample_count_num * ctx->trigger_count_num > SCPIMM_BUF_CAPACITY) {
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

scpimm_mode_params_t* SCPIMM_mode_params(scpimm_context_t* const ctx, const scpimm_mode_t mode) {
	switch (mode) {
		case SCPIMM_MODE_DCV:
			return &ctx->mode_params.dcv;

		case SCPIMM_MODE_DCV_RATIO:
			return &ctx->mode_params.dcv_ratio;

		case SCPIMM_MODE_ACV:
			return &ctx->mode_params.acv;

		case SCPIMM_MODE_DCC:
			return &ctx->mode_params.dcc;

		case SCPIMM_MODE_ACC:
			return &ctx->mode_params.acc;

		case SCPIMM_MODE_RESISTANCE_2W:
			return &ctx->mode_params.resistance;

		case SCPIMM_MODE_RESISTANCE_4W:
			return &ctx->mode_params.fresistance;

		case SCPIMM_MODE_FREQUENCY:
			return &ctx->mode_params.frequency;

		case SCPIMM_MODE_PERIOD:
			return &ctx->mode_params.period;
	}

	return NULL;
}

int16_t SCPIMM_set_mode(scpi_t* const context, const scpimm_mode_t mode, const scpi_number_t* const range, const bool_t auto_detect_auto_range, const bool_t* const auto_range, const scpi_number_t* const resolution) {
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
	const scpimm_interface_t* const intf = ctx->interface;
	scpimm_mode_params_t* const ctx_params = SCPIMM_mode_params(ctx, mode);
	scpimm_mode_params_t new_params;
	double min_value, max_value;
	int16_t err;

	if (ctx_params) {
		/* given mode needs parameters: range, resolution etc */
		new_params = *ctx_params;

		if (range) {
			switch (range->type) {
			case SCPI_NUM_NUMBER:
				CHECK_SCPI_ERROR(intf->get_possible_range(mode, &min_value, &max_value));
				if (range->value < min_value) {
					new_params.range = min_value;
				} else if (range->value > max_value * INCREASE_DELTA) {
					return SCPI_ERROR_DATA_OUT_OF_RANGE;
				} else {
					new_params.range = range->value;
				}
				if (auto_detect_auto_range) {
					new_params.auto_range = FALSE;
				}
				break;

			case SCPI_NUM_MIN:
				CHECK_SCPI_ERROR(intf->get_possible_range(mode, &new_params.range, NULL));
				if (auto_detect_auto_range) {
					new_params.auto_range = FALSE;
				}
				break;

			case SCPI_NUM_MAX:
				CHECK_SCPI_ERROR(intf->get_possible_range(mode, NULL, &new_params.range));
				if (auto_detect_auto_range) {
					new_params.auto_range = FALSE;
				}
				break;

			case SCPI_NUM_DEF:
				CHECK_SCPI_ERROR(intf->get_possible_range(mode, &new_params.range, NULL));
				new_params.auto_range = TRUE;
				break;

			default:
				return SCPI_ERROR_ILLEGAL_PARAMETER_VALUE;
			}
		}

		if (auto_range) {
			new_params.auto_range = *auto_range;
		}

		if (resolution) {
			switch (resolution->type) {
			case SCPI_NUM_NUMBER:
				CHECK_SCPI_ERROR(intf->get_possible_resolution(mode, new_params.range, &min_value, &max_value));
				if (resolution->value < min_value * DECREASE_DELTA) {
					return SCPI_ERROR_CANNOT_ACHIEVE_REQUESTED_RESOLUTION;
				} else if (resolution->value > max_value) {
					new_params.resolution = max_value;
				} else {
					new_params.resolution = resolution->value;
				}
				break;

			case SCPI_NUM_DEF:
			case SCPI_NUM_MIN:
				CHECK_SCPI_ERROR(intf->get_possible_resolution(mode, new_params.range, &new_params.resolution, NULL));
				break;

			case SCPI_NUM_MAX:
				CHECK_SCPI_ERROR(intf->get_possible_resolution(mode, new_params.range, NULL, &new_params.resolution));
				break;

			default:
				return SCPI_ERROR_ILLEGAL_PARAMETER_VALUE;
			}
		}
	}

	// set mode and parameters
	CHECK_SCPI_ERROR(intf->set_mode(mode, ctx_params ? &new_params : NULL));

	if (ctx_params) {
		// read actual parameters
		CHECK_SCPI_ERROR(intf->get_mode(NULL, ctx_params));
	}

	return SCPI_ERROR_OK;
}
