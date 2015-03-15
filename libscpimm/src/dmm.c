#include <scpimm/scpimm.h>
#include "dmm.h"
#include "utils.h"
#include "input.h"

#define SCPIMM_OVERFLOW 9.90000000E+37
#define	MEASUREMENT_TIMEOUT (10 * 1000)

static int16_t switch_to_timed_state(volatile scpimm_context_t* const ctx, const scpimm_state_t new_state) {
	uint32_t tm;
	int16_t err;

	CHECK_SCPI_ERROR(ctx->interface->get_milliseconds(&tm));
	ctx->state_time = tm;

	ATOMIC_WRITE_INT(ctx->state, new_state);

	return SCPI_ERROR_OK;
}

static void switch_to_state(volatile scpimm_context_t* const ctx, const scpimm_state_t new_state) {
	ATOMIC_WRITE_INT(ctx->state, new_state);
}

static int16_t start_measurement(volatile scpimm_context_t* const ctx) {
	int16_t err;
	uint32_t cur_time;

	ctx->last_measured_value.type = SCPI_NUM_INF;
	CHECK_SCPI_ERROR(ctx->interface->get_milliseconds(&cur_time));
	ctx->measure_start_time = cur_time;

	CHECK_SCPI_ERROR(switch_to_timed_state(ctx, SCPIMM_STATE_MEASURING));
	CHECK_SCPI_ERROR(ctx->interface->start_measure());

	return SCPI_ERROR_OK;
}

static int16_t trigger_pulled(volatile scpimm_context_t* const ctx) {
	return switch_to_timed_state(ctx, SCPIMM_STATE_TRIGGER_DELAY);
}

static int16_t init_trigger(volatile scpimm_context_t* const ctx) {
	int16_t err = SCPI_ERROR_OK;

	switch_to_state(ctx, SCPIMM_STATE_WAIT_FOR_TRIGGER);

	switch (ctx->trigger_src) {
	case SCPIMM_TRIG_IMM:
		err = trigger_pulled(ctx);
		break;

	case SCPIMM_TRIG_BUS:
		if (SCPIMM_DST_OUT == ctx->dst) {
			return SCPI_ERROR_TRIGGER_DEADLOCK;
		}
		// will wait for *TRG command
		break;

	case SCPIMM_TRIG_EXT:
		// will wait for external trigger
		break;
	}

	return err;
}

static int16_t initiate(volatile scpimm_context_t* const ctx, const scpimm_dst_t dst) {
	int16_t err;

	if (SCPIMM_STATE_IDLE != ATOMIC_READ_INT(ctx->state)) {
		return SCPI_ERROR_INIT_IGNORED;
	}

	ctx->dst = dst;
	ctx->buf_count = 0;
	ctx->sample_count = ctx->sample_count_num;
	ctx->trigger_count = ctx->trigger_count_num;
	ctx->measurement_error = SCPI_ERROR_OK;
	ctx->is_first_measured_value = TRUE;

	err = init_trigger(ctx);
	if (SCPI_ERROR_OK != err) {
		switch_to_state(ctx, SCPIMM_STATE_IDLE);
	}

	return err;
}

static int16_t wait_for_idle(volatile scpimm_context_t* const ctx) {
	int16_t err;

	while (SCPIMM_STATE_IDLE != ATOMIC_READ_INT(ctx->state)) {
		SCPIMM_yield();
	}

	err = ctx->measurement_error;
	ctx->measurement_error = SCPI_ERROR_OK;

	return err;
}

scpi_result_t SCPIMM_measure_preset(scpi_t* context) {
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);

	ATOMIC_WRITE_INT(ctx->state, SCPIMM_STATE_IDLE);
	ctx->sample_count_num = 1;
	ctx->trigger_count_num = 1;
	ctx->trigger_delay = 0;
	ctx->trigger_auto_delay = TRUE;
	ctx->infinite_trigger_count = FALSE;
	ctx->trigger_src = SCPIMM_TRIG_IMM;
	ctx->buf_count = 0;
	ctx->measurement_timeout = MEASUREMENT_TIMEOUT;

	/* TODO
AC Filter (DET:BAND)                20 Hz (medium filter)
Autozero (ZERO:AUTO)                OFF if resolution setting results in NPLC < 1;
                                    ON if resolution setting results in NPLC ≥ 1
                                    OFF (fixed at 10 MΩ for all dc voltage ranges)
Math Function (CALCulate subsystem) OFF
	*/

	return SCPIMM_do_set_input_impedance_auto(context, FALSE);
}

scpimm_state_t SCPIMM_get_state(scpi_t* context) {
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
	const scpimm_state_t state = ATOMIC_READ_INT(ctx->state);
	return state;
}

static int16_t check_trigger_delay(volatile scpimm_context_t* const ctx) {
	int16_t err;
	uint32_t cur_time;

	CHECK_SCPI_ERROR(ctx->interface->get_milliseconds(&cur_time));

	if (cur_time - ctx->state_time >= (uint32_t) ctx->trigger_delay) {
		// trigger delay exhausted
		CHECK_SCPI_ERROR(start_measurement(ctx));
	}

	return SCPI_ERROR_OK;
}

static int16_t check_measuring_timeout(volatile scpimm_context_t* const ctx) {
	int16_t err;
	uint32_t cur_time;

	CHECK_SCPI_ERROR(ctx->interface->get_milliseconds(&cur_time));

	if (cur_time - ctx->state_time >= ctx->measurement_timeout) {
		// measure timeout
		return SCPI_ERROR_IO_PROCESSOR_DOES_NOT_RESPOND;
	}

	return SCPI_ERROR_OK;
}

static int16_t store_value_in_buffer(volatile scpimm_context_t* const ctx, const double value) {
	if (SCPIMM_BUF_LEN > ctx->buf_count) {
		ctx->buf[ctx->buf_count++] = value;
		return SCPI_ERROR_OK;
	} else {
		return SCPI_ERROR_INSUFFICIENT_MEMORY;
	}
}

static int16_t send_value_to_out_buffer(volatile scpimm_context_t* const ctx, const double value) {
	char buf[16];
	size_t len;
	scpi_t* const context = SCPI_context();

	if (ctx->is_first_measured_value) {
		ctx->is_first_measured_value = FALSE;
	} else {
		context->interface->write(context, ",", 1);
	}

	len = double_to_str(buf, value);
	context->interface->write(context, buf, len);

	return SCPI_ERROR_OK;
}

static void flush_out_buffer() {
	scpi_t* const context = SCPI_context();
	context->interface->write(context, "\r\n", 2);
}

static int16_t check_measured_value(volatile scpimm_context_t* const ctx) {
	int16_t err;
	const double value = ctx->last_measured_value.type == SCPI_NUM_NUMBER
			? ctx->last_measured_value.value
			: SCPIMM_OVERFLOW;

	switch (ctx->dst) {
	case SCPIMM_DST_BUF:
		CHECK_SCPI_ERROR(store_value_in_buffer(ctx, value));
		break;

	case SCPIMM_DST_OUT:
		CHECK_SCPI_ERROR(send_value_to_out_buffer(ctx, value));
		break;
	}

	if (0 == --ctx->sample_count) {
		if (!ctx->infinite_trigger_count && 0 == --ctx->trigger_count) {
			if (SCPIMM_DST_OUT == ctx->dst) {
				flush_out_buffer();
			}
			switch_to_state(ctx, SCPIMM_STATE_IDLE);
		} else {
			ctx->sample_count = ctx->sample_count_num;
			CHECK_SCPI_ERROR(init_trigger(ctx));
		}
	} else {
		CHECK_SCPI_ERROR(switch_to_timed_state(ctx, SCPIMM_STATE_TRIGGER_DELAY));
	}

	return SCPI_ERROR_OK;
}

void SCPIMM_yield() {
	volatile scpimm_context_t* const ctx = SCPIMM_context();
	int16_t err = SCPI_ERROR_OK;

	switch (ATOMIC_READ_INT(ctx->state)) {
	case SCPIMM_STATE_IDLE:
	case SCPIMM_STATE_WAIT_FOR_TRIGGER:
		// nothing to do
		break;

	case SCPIMM_STATE_TRIGGER_DELAY:
		err = check_trigger_delay(ctx);
		break;

	case SCPIMM_STATE_MEASURING:
		err = check_measuring_timeout(ctx);
		break;

	case SCPIMM_STATE_MEASURED:
		err = check_measured_value(ctx);
		break;
	}

	if (SCPI_ERROR_OK != err) {
		switch_to_state(ctx, SCPIMM_STATE_IDLE);
		ctx->measurement_error = err;
	}
}

void SCPIMM_read_value(const scpi_number_t* value) {
	volatile scpimm_context_t* const ctx = SCPIMM_context();

	ctx->last_measured_value.type = SCPI_NUM_NUMBER;
	ctx->last_measured_value.value = SCPI_NUM_NUMBER == value->type ? value->value : SCPIMM_OVERFLOW;

	if (SCPIMM_STATE_MEASURING != ATOMIC_READ_INT(ctx->state)) {
		/* measurement is not expected now */
		return;
	}

	switch_to_state(ctx, SCPIMM_STATE_MEASURED);
}

scpi_result_t SCPIMM_initiate(scpi_t* context) {
	int16_t err;
	volatile scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);

	if (	ctx->infinite_trigger_count
			|| ctx->sample_count_num * ctx->trigger_count_num > SCPIMM_BUF_CAPACITY) {
	    SCPI_ErrorPush(context, SCPI_ERROR_INSUFFICIENT_MEMORY);
		return SCPI_RES_ERR;
	}

	CHECK_AND_PUSH_ERROR(initiate(ctx, SCPIMM_DST_BUF));

	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_readQ(scpi_t* context) {
	volatile scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
	int16_t err;

	CHECK_AND_PUSH_ERROR(initiate(ctx, SCPIMM_DST_OUT));
	CHECK_AND_PUSH_ERROR(wait_for_idle(ctx));

	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_fetchQ(scpi_t* context) {
	volatile scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
	int16_t err;
	scpi_result_t result = SCPI_RES_OK;
	size_t i;

	CHECK_AND_PUSH_ERROR(wait_for_idle(ctx));

	if (!ctx->buf_count) {
		/* no data to transfer */
		CHECK_AND_PUSH_ERROR(SCPI_ERROR_DATA_STALE);
	}

	for (i = 0; i < ctx->buf_count; i++) {
		// TODO valid delimiters
		SCPIMM_ResultDouble(context, ctx->buf[i]);
	}

	return result;
}

scpi_result_t SCPIMM_trg(scpi_t* const context) {
	int16_t err;
	volatile scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
	
	if (	SCPIMM_STATE_WAIT_FOR_TRIGGER != ATOMIC_READ_INT(ctx->state)
			|| SCPIMM_TRIG_BUS != ctx->trigger_src ) {
	    SCPI_ErrorPush(context, SCPI_ERROR_TRIGGER_IGNORED);
		return SCPI_RES_ERR;
	}

	CHECK_AND_PUSH_ERROR(trigger_pulled(ctx));

	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_data_pointsQ(scpi_t* context) {
	volatile scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
	SCPI_ResultInt(context, ctx->buf_count);
	return SCPI_RES_OK;
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

void SCPIMM_external_trigger() {
	volatile scpimm_context_t* const ctx = SCPIMM_context();
	if (	SCPIMM_STATE_WAIT_FOR_TRIGGER == ATOMIC_READ_INT(ctx->state)
			&& SCPIMM_TRIG_EXT == ctx->trigger_src ) {

		trigger_pulled(ctx);
	}
}
