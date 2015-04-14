/*
	This file is part of scpi-multimeter library.

	v7-28-arduino is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	v7-28-arduino is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with v7-28-arduino.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <scpimm/scpimm.h>
#include "trigger.h"
#include "scpimm_internal.h"
#include "utils.h"

#define	INF_TRIGGER_COUNT	9.90000000E+37

#define	TRIGGER_DELAY_MIN	0.0
#define	TRIGGER_DELAY_MAX	3600.0

static const char* options[] = {"BUS", "IMMediate", "EXTernal", NULL};

static long max_trigger_count() {
	return MAX_TRIGGER_COUNT;
}

scpi_result_t SCPIMM_trigger_source(scpi_t* context) {
	static const scpimm_trig_src_t values[] = {SCPIMM_TRIG_BUS, SCPIMM_TRIG_IMM, SCPIMM_TRIG_EXT};
	int32_t choice;

    if (!SCPI_ParamChoice(context, options, &choice, TRUE)) {
		return SCPI_RES_ERR;
	}

	SCPIMM_CONTEXT(context)->trigger_src = values[choice];
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_trigger_sourceQ(scpi_t* context) {
	const char* res = NULL;

	switch (SCPIMM_CONTEXT(context)->trigger_src) {
		case SCPIMM_TRIG_BUS:
			res = "BUS";
			break;

		case SCPIMM_TRIG_IMM:
			res = "IMM";
			break;

		case SCPIMM_TRIG_EXT:
			res = "EXT";
			break;
	}

	SCPI_ResultString(context, res);
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_trigger_delay(scpi_t* context) {
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
	scpi_number_t num;

    if (!SCPI_ParamNumber(context, &num, TRUE)) {
		return SCPI_RES_ERR;
	}

	switch (num.type) {
		case SCPI_NUM_MIN:
			ctx->trigger_delay = TRIGGER_DELAY_MIN;
			break;

		case SCPI_NUM_MAX:
			ctx->trigger_delay = TRIGGER_DELAY_MAX;
			break;

		case SCPI_NUM_NUMBER:
			if (num.value < TRIGGER_DELAY_MIN || num.value > TRIGGER_DELAY_MAX) {
				SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
				return SCPI_RES_ERR;
			}
			ctx->trigger_delay = num.value;
			break;

		default:
			SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
			return SCPI_RES_ERR;
	}

	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_trigger_delayQ(scpi_t* context) {
    scpi_number_t num;
	double res;

    if (!SCPI_ParamNumber(context, &num, FALSE)) {
		return SCPI_RES_ERR;
	}

	switch (num.type) {
		case SCPI_NUM_MIN:
			res = TRIGGER_DELAY_MIN;
			break;

		case SCPI_NUM_MAX:
			res = TRIGGER_DELAY_MAX;
			break;

		case SCPI_NUM_DEF:
			res = SCPIMM_CONTEXT(context)->trigger_delay;
			break;

		default:
			SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
			return SCPI_RES_ERR;
	}

	SCPIMM_ResultDouble(context, res);
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_trigger_delay_auto(scpi_t* context) {
	scpi_bool_t b;

    if (!SCPI_ParamBool(context, &b, TRUE)) {
		return SCPI_RES_ERR;
	}

	SCPIMM_CONTEXT(context)->trigger_auto_delay = b;
    return SCPI_RES_OK;
}

scpi_result_t SCPIMM_trigger_delay_autoQ(scpi_t* context) {
	SCPI_ResultBool(context, SCPIMM_CONTEXT(context)->trigger_auto_delay);
    return SCPI_RES_OK;
}

scpi_result_t SCPIMM_trigger_count(scpi_t* context) {
	const long lmax = max_trigger_count();
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
    scpi_number_t num;
	long lnum;

    if (!SCPI_ParamNumber(context, &num, TRUE)) {
		return SCPI_RES_ERR;
	}
	
	switch (num.type) {
		case SCPI_NUM_MIN:
			lnum = 1;
			ctx->infinite_trigger_count = FALSE;
			break;

		case SCPI_NUM_MAX:
			lnum = lmax;
			ctx->infinite_trigger_count = FALSE;
			break;

		case SCPI_NUM_INF:
			lnum = 0;
			ctx->infinite_trigger_count = TRUE;
			break;

		case SCPI_NUM_NUMBER:
			lnum = (long) num.value;
			ctx->infinite_trigger_count = FALSE;
			break;

		default:
			SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
			return SCPI_RES_ERR;
	}

	if (!ctx->infinite_trigger_count && (lnum < 1 || lnum > lmax)) {
	    SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
    	return SCPI_RES_ERR;
	}

	ctx->trigger_count_num = (unsigned) lnum;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_trigger_countQ(scpi_t* context) {
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
    scpi_number_t num = {0.0, SCPI_UNIT_NONE, SCPI_NUM_NUMBER};
	int32_t lnum;

    if (!SCPI_ParamNumber(context, &num, FALSE)) {
		return SCPI_RES_ERR;
	}

	switch (num.type) {
		case SCPI_NUM_MIN:
			lnum = 1;
			break;

		case SCPI_NUM_MAX:
			lnum = max_trigger_count();
			break;

		case SCPI_NUM_DEF:
			if (ctx->infinite_trigger_count) {
				SCPIMM_ResultDouble(context, INF_TRIGGER_COUNT);
				return SCPI_RES_OK;
			}
			lnum = ctx->trigger_count_num;
			break;

		default:
			SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
			return SCPI_RES_ERR;
	}

	SCPI_ResultInt(context, lnum);
	return SCPI_RES_OK;
}

