/*
    Copyright (c) 2015 Andrey Nakin
    All Rights Reserved

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
#include "sample.h"
#include "scpimm_internal.h"
#include "utils.h"

static long max_sample_count() {
	return MAX_SAMPLE_COUNT;
}

scpi_result_t SCPIMM_sample_count(scpi_t* context) {
	const long lmax = max_sample_count();
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
    scpi_number_t num;
	long lnum;

    if (!SCPI_ParamNumber(context, &num, TRUE)) {
		return SCPI_RES_ERR;
	}
	
	switch (num.type) {
		case SCPI_NUM_MIN:
			lnum = 1;
			break;

		case SCPI_NUM_MAX:
			lnum = lmax;
			break;

		case SCPI_NUM_NUMBER:
			lnum = (long) num.value;
			break;

		default:
			SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
			return SCPI_RES_ERR;
	}

	if (lnum < 1 || lnum > lmax) {
	    SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
    	return SCPI_RES_ERR;
	}

	ctx->sample_count_num = (uint16_t) lnum;
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_sample_countQ(scpi_t* context) {
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
			lnum = max_sample_count();
			break;

		case SCPI_NUM_DEF:
			lnum = ctx->sample_count_num;
			break;

		default:
			SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
			return SCPI_RES_ERR;
	}

	SCPI_ResultInt(context, lnum);
	return SCPI_RES_OK;
}

