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

static scpi_result_t wait_for_idle(scpi_t* const context) {
	while (SCPIMM_STATE_IDLE != SCPIMM_get_state(context)) {
		SCPIMM_INTERFACE(context)->sleep_milliseconds(50);
	}

	return SCPI_RES_OK;
}

static scpi_result_t start_measure(scpi_t* const context) {
	scpi_result_t result;

	if (SCPI_RES_OK == (result = SCPIMM_set_state(context, SCPIMM_STATE_MEASURE))) {
		const int16_t err = SCPIMM_INTERFACE(context)->start_measure();
		if (err) {
		    SCPI_ErrorPush(context, err);
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
			SCPIMM_ResultDouble(context, v);
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

	SCPIMM_ResultDouble(context, ctx->buf[ctx->buf_head++]);
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

static size_t max_index(const double* values) {
	size_t result;

	for (result = 0; values[result] >= 0; result++) {}

	return --result;
}

static int find_greater_than(const double* values, const double v) {
	int result;

	for (result = 0; values[result] >= 0; result++) {
		if (values[result] > v) {
			return result;
		}
	}

	return -1;
}

int16_t SCPIMM_set_mode(scpi_t* const context, const scpimm_mode_t mode, const scpi_number_t* const range, const bool_t auto_detect_auto_range, const bool_t* const auto_range, const scpi_number_t* const resolution) {
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
	const scpimm_interface_t* const intf = ctx->interface;
	scpimm_mode_params_t* const ctx_params = SCPIMM_mode_params(ctx, mode);
	scpimm_mode_params_t new_params;
	int16_t err;

	if (ctx_params) {
		/* given mode needs parameters: range, resolution etc */
		new_params = *ctx_params;

		if (range) {
			const double* ranges, *overruns;

			new_params.range_index = 0;
			CHECK_SCPI_ERROR(intf->get_allowed_ranges(mode, &ranges, &overruns));

			switch (range->type) {
			case SCPI_NUM_NUMBER:
				for (; ranges[new_params.range_index] >= 0.0; new_params.range_index++) {
					if (range->value <= ranges[new_params.range_index] * overruns[new_params.range_index]) {
						break;
					}
				}

				if (ranges[new_params.range_index] < 0) {
					return SCPI_ERROR_DATA_OUT_OF_RANGE;
				}

				if (auto_detect_auto_range) {
					new_params.auto_range = FALSE;
				}
				break;

			case SCPI_NUM_MIN:
				if (auto_detect_auto_range) {
					new_params.auto_range = FALSE;
				}
				break;

			case SCPI_NUM_MAX:
				new_params.range_index = max_index(ranges);

				if (auto_detect_auto_range) {
					new_params.auto_range = FALSE;
				}
				break;

			case SCPI_NUM_DEF:
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
			const double* resolutions;

			new_params.resolution_index = 0;
			CHECK_SCPI_ERROR(intf->get_allowed_resolutions(mode, new_params.range_index, &resolutions));

			switch (resolution->type) {
			case SCPI_NUM_NUMBER: {
				const int idx = find_greater_than(resolutions, resolution->value * (1.0 + 1.e-6));
				if (idx < 0) {
					new_params.resolution_index = max_index(resolutions);
				} else if (idx > 0) {
					new_params.resolution_index = idx - 1;
				} else {
					return SCPI_ERROR_CANNOT_ACHIEVE_REQUESTED_RESOLUTION;
				}
			}
				break;

			case SCPI_NUM_DEF:
			case SCPI_NUM_MIN:
				break;

			case SCPI_NUM_MAX:
				new_params.resolution_index = max_index(resolutions);
				break;

			default:
				return SCPI_ERROR_ILLEGAL_PARAMETER_VALUE;
			}
		}
	}

	// set mode and parameters
	CHECK_SCPI_ERROR(intf->set_mode(mode, ctx_params ? &new_params : NULL));

	if (ctx_params) {
		*ctx_params = new_params;
	}

	return SCPI_ERROR_OK;
}

const char* SCPIMM_mode_name(const scpimm_mode_t mode) {
	const char* res;

	switch (mode) {
		case SCPIMM_MODE_DCV:
			res = "VOLT";
			break;

		case SCPIMM_MODE_DCV_RATIO:
			res = "VOLT:DC:RAT";	/* TODO */
			break;

		case SCPIMM_MODE_ACV:
			res = "VOLT:AC";
			break;

		case SCPIMM_MODE_DCC:
			res = "CURR";
			break;

		case SCPIMM_MODE_ACC:
			res = "CURR:AC";
			break;

		case SCPIMM_MODE_RESISTANCE_2W:
			res = "RES";
			break;

		case SCPIMM_MODE_RESISTANCE_4W:
			res = "FRES";
			break;

		case SCPIMM_MODE_FREQUENCY:
			res = "FREQ";
			break;

		case SCPIMM_MODE_PERIOD:
			res = "PER";
			break;

		case SCPIMM_MODE_CONTINUITY:
			res = "CONT";
			break;

		case SCPIMM_MODE_DIODE:
			res = "DIOD";
			break;

		default:
			res = NULL;
    }

	return res;
}
