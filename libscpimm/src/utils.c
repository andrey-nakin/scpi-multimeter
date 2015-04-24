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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <scpi/config.h>
#include "scpimm_internal.h"
#include "utils.h"

scpi_bool_t expectNoParams(scpi_t* context) {
    const char * param;
    size_t param_len;

	if (SCPI_ParamString(context, &param, &param_len, FALSE)) {
	    SCPI_ErrorPush(context, SCPIMM_ERROR_PARAMETER_NOT_ALLOWED);
		return FALSE;
	}

    return TRUE;
}

size_t min_value_index(const double* const values) {
	size_t result = SIZE_MAX;
	double min_value = 0.0;
	const double* v = values;

	for (; *v >= 0.0; v++) {
		if (v == values || min_value > *v) {
			min_value = *v;
			result = v - values;;
		}
	}

	return result;
}

size_t max_value_index(const double* const values) {
	size_t result = SIZE_MAX;
	double max_value = 0.0;
	const double* v = values;

	for (; *v >= 0.0; v++) {
		if (v == values || max_value < *v) {
			max_value = *v;
			result = v - values;;
		}
	}

	return result;
}

size_t less_or_equal_index(const double* values, const double value) {
	size_t result = SIZE_MAX;
	double max_value = 0.0;
	const double* v = values;

	for (; *v >= 0.0; v++) {
		if (*v <= value) {
			if (SIZE_MAX == result || max_value < *v) {
				max_value = *v;
				result = v - values;;
			}
		}
	}

	return result;
}

size_t greater_or_equal_index(const double* values, const double v) {
	size_t result;

	for (result = 0; values[result] >= 0; result++) {
		if (values[result] >= v) {
			return result;
		}
	}

	return SIZE_MAX;
}
