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
#include <string.h>
#include "CUnit/Basic.h"
#include "test_utils.h"

#define	LAST_PREFIX	NULL
#define	LAST_SUFFIX	NULL

typedef struct {
	scpimm_bool_param_t param;
	const char* suffixes[16];
} bool_param_context_t;

typedef struct {
	scpimm_numeric_param_t param;
	const char* suffixes[16];
	scpi_bool_t min_max;
} numeric_param_context_t;

static const char* const SENSE_PREFIXES[] = {"SENSE:", "", LAST_PREFIX};
static const char* const MIN_VALUES[] = {"MINimum", "MIN", NULL};
static const char* const MAX_VALUES[] = {"MAXimum", "MAX", NULL};

static const bool_param_context_t RANGE_AUTO_CONTEXT = {
	.param = SCPIMM_PARAM_RANGE_AUTO,
	.suffixes = {"RANGe:AUTO", "RANG:AUTO", LAST_SUFFIX}
};

static const numeric_param_context_t NPLC_CONTEXT = {
	.param = SCPIMM_PARAM_NPLC,
	.suffixes = {"NPLCycles", "NPLC", LAST_SUFFIX},
	.min_max = TRUE
};

/*****************************************************************************
 * SENSE:<function>:<bool parameter>
*****************************************************************************/

static void test_set_bool_param_impl3(const char* const prefix, const char* const func, const char* const suffix, const scpimm_mode_t mode, const scpimm_bool_param_t param, const char* const value, const scpi_bool_t expected) {
	dm_reset_counters();
	dm_reset_args();
	receivef("%s%s:%s %s", prefix, func, suffix, value);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(dm_counters.set_bool_param, CALLED_ONCE);
	CU_ASSERT_EQUAL(dm_args.set_bool_param.mode, mode);
	CU_ASSERT_EQUAL(dm_args.set_bool_param.param, param);
	CU_ASSERT_EQUAL(dm_args.set_bool_param.value, expected);
}

static void test_set_bool_param_impl2(const char* const prefix, const char* const func, const scpimm_mode_t mode, const bool_param_context_t* const context) {
	static const char* const values[] = {"ON", "OFF", "1", "0", NULL};
	static scpi_bool_t const expected[] = {TRUE, FALSE, TRUE, FALSE, NULL};
	const char* const* suffix;

	for (suffix = context->suffixes; *suffix; suffix++) {
		size_t value_index;

		for (value_index = 0; values[value_index]; value_index++) {
			test_set_bool_param_impl3(prefix, func, *suffix, mode, context->param, values[value_index], expected[value_index]);
		}
	}
}

static void test_set_bool_param_impl(const char* const func, const scpimm_mode_t mode, void* user_data) {
	const bool_param_context_t* const context = (const bool_param_context_t*) user_data;
	const char* const* prefix;

	if (SCPIMM_MODE_DCV_RATIO == mode || SCPIMM_MODE_ACV_RATIO == mode) {
		return;
	}

	for (prefix = SENSE_PREFIXES; *prefix; prefix++) {
		test_set_bool_param_impl2(*prefix, func, mode, context);
	}
}

/*****************************************************************************
 * SENSE:<function>:<bool parameter>?
*****************************************************************************/

static void test_get_bool_param_impl3(const char* const prefix, const char* const func, const char* const suffix, const scpimm_mode_t mode, const scpimm_bool_param_t param, const char* const value, const scpi_bool_t expected) {
	receivef("%s%s:%s %s", prefix, func, suffix, value);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();

	dm_reset_counters();
	dm_reset_args();
	receivef("%s%s:%s?", prefix, func, suffix);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_BOOL_RESPONSE(expected);
	CU_ASSERT_EQUAL(dm_counters.get_bool_param, CALLED_ONCE);
	CU_ASSERT_EQUAL(dm_args.get_bool_param.mode, mode);
	CU_ASSERT_EQUAL(dm_args.get_bool_param.param, param);
	CU_ASSERT_EQUAL(dm_args.get_bool_param.value_is_null, FALSE);
}

static void test_get_bool_param_impl2(const char* const prefix, const char* const func, const scpimm_mode_t mode, const bool_param_context_t* const context) {
	static const char* const values[] = {"ON", "OFF", "1", "0", NULL};
	static scpi_bool_t const expected[] = {TRUE, FALSE, TRUE, FALSE, NULL};
	const char* const* suffix;

	for (suffix = context->suffixes; *suffix; suffix++) {
		size_t value_index;

		for (value_index = 0; values[value_index]; value_index++) {
			test_get_bool_param_impl3(prefix, func, *suffix, mode, context->param, values[value_index], expected[value_index]);
		}
	}
}

static void test_get_bool_param_impl(const char* const func, const scpimm_mode_t mode, void* user_data) {
	const bool_param_context_t* const context = (const bool_param_context_t*) user_data;
	const char* const* prefix;

	if (SCPIMM_MODE_DCV_RATIO == mode || SCPIMM_MODE_ACV_RATIO == mode) {
		return;
	}

	for (prefix = SENSE_PREFIXES; *prefix; prefix++) {
		test_get_bool_param_impl2(*prefix, func, mode, context);
	}
}

/*****************************************************************************
 * SENSE:<function>:<numeric parameter>
*****************************************************************************/

static void test_set_numeric_param_impl2(const char* const prefix, const char* const func, const char* const suffix, const scpimm_mode_t mode, const scpimm_numeric_param_t param, const char* const value, const size_t expected) {
	dm_reset_counters();
	dm_reset_args();
	receivef("%s%s:%s %s", prefix, func, suffix, value);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(dm_counters.set_numeric_param, CALLED_ONCE);
	CU_ASSERT_EQUAL(dm_args.set_numeric_param.mode, mode);
	CU_ASSERT_EQUAL(dm_args.set_numeric_param.param, param);
	CU_ASSERT_EQUAL(dm_args.set_numeric_param.value_index, expected);
}

static void test_set_numeric_param_impl(const char* const func, const scpimm_mode_t mode, void* user_data) {
	const numeric_param_context_t* const context = (const numeric_param_context_t*) user_data;
	const char* const* prefix;
	const double* values;

	if (SCPIMM_MODE_DCV_RATIO == mode) {
		return;
	}

	{
		int16_t err;
		const scpimm_interface_t* const intf = scpimm_interface();
		CHECK_NO_SCPI_ERROR(intf->get_numeric_param_values(mode, context->param, &values));
	}

	for (prefix = SENSE_PREFIXES; *prefix; prefix++) {
		const char* const* suffix;

		for (suffix = context->suffixes; *suffix; suffix++) {
			size_t value_index;

			if (context->min_max) {
				const char* const* value;

				value_index = min_value_index(values);
				for (value = MIN_VALUES; *value; value++) {
					test_set_numeric_param_impl2(*prefix, func, *suffix, mode, context->param, *value, value_index);
				}

				value_index = max_value_index(values);
				for (value = MAX_VALUES; *value; value++) {
					test_set_numeric_param_impl2(*prefix, func, *suffix, mode, context->param, *value, value_index);
				}
			}

			for (value_index = 0; values[value_index] >= 0.0; value_index++) {
				char value[64];

				sprintf(value, "%0.6g", values[value_index]);
				test_set_numeric_param_impl2(*prefix, func, *suffix, mode, context->param, value, value_index);

				sprintf(value, "%0.6g", 0.9 * values[value_index]);
				test_set_numeric_param_impl2(*prefix, func, *suffix, mode, context->param, value, value_index);
			}
		}
	}
}

/*****************************************************************************
 * SENSE:<function>:<numeric parameter>?
*****************************************************************************/

static void test_get_numeric_param_impl2(const char* const prefix, const char* const func, const char* const suffix, const scpimm_mode_t mode, const scpimm_numeric_param_t param, const char* const value, const double expected) {
	receivef("%s%s:%s %s", prefix, func, suffix, value);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();

	dm_reset_counters();
	dm_reset_args();
	receivef("%s%s:%s?", prefix, func, suffix, value);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_DOUBLE_RESPONSE(expected);
	CU_ASSERT_EQUAL(dm_counters.get_numeric_param, CALLED_ONCE);
	CU_ASSERT_EQUAL(dm_args.get_numeric_param.mode, mode);
	CU_ASSERT_EQUAL(dm_args.get_numeric_param.param, param);
	CU_ASSERT_EQUAL(dm_args.get_numeric_param.value_is_null, FALSE);
}

static void test_get_numeric_param_min_max(const char* const prefix, const char* const func, const char* const suffix, const scpimm_mode_t mode, const scpimm_numeric_param_t param, const char* const value, const double expected) {
	dm_reset_counters();
	dm_reset_args();
	receivef("%s%s:%s? %s", prefix, func, suffix, value);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_DOUBLE_RESPONSE(expected);
	CU_ASSERT_EQUAL(dm_counters.get_numeric_param, NOT_CALLED);
	CU_ASSERT_EQUAL(dm_counters.get_numeric_param_values, CALLED_ONCE);
	CU_ASSERT_EQUAL(dm_args.get_numeric_param_values.mode, mode);
	CU_ASSERT_EQUAL(dm_args.get_numeric_param_values.param, param);
	CU_ASSERT_EQUAL(dm_args.get_numeric_param_values.values_is_null, FALSE);
}

static void test_get_numeric_param_impl(const char* const func, const scpimm_mode_t mode, void* user_data) {
	const numeric_param_context_t* const context = (const numeric_param_context_t*) user_data;
	const char* const* prefix;
	const double* values;

	if (SCPIMM_MODE_DCV_RATIO == mode) {
		return;
	}

	{
		int16_t err;
		const scpimm_interface_t* const intf = scpimm_interface();
		CHECK_NO_SCPI_ERROR(intf->get_numeric_param_values(mode, context->param, &values));
	}

	for (prefix = SENSE_PREFIXES; *prefix; prefix++) {
		const char* const* suffix;

		for (suffix = context->suffixes; *suffix; suffix++) {
			size_t value_index;

			if (context->min_max) {
				const char* const* value;

				value_index = min_value_index(values);
				for (value = MIN_VALUES; *value; value++) {
					test_get_numeric_param_min_max(*prefix, func, *suffix, mode, context->param, *value, values[value_index]);
				}

				value_index = max_value_index(values);
				for (value = MAX_VALUES; *value; value++) {
					test_get_numeric_param_min_max(*prefix, func, *suffix, mode, context->param, *value, values[value_index]);
				}
			}

			for (value_index = 0; values[value_index] >= 0.0; value_index++) {
				char value[64];
				sprintf(value, "%0.6g", values[value_index]);
				test_get_numeric_param_impl2(*prefix, func, *suffix, mode, context->param, value, values[value_index]);
			}
		}
	}
}

/*****************************************************************************
 * SENSE:<function>
*****************************************************************************/

static void test_function_impl2(const char* const prefix, const char* const func, const scpimm_mode_t mode) {
	dm_reset_counters();
	dm_reset_args();

	printf("### [[[%sFUNCTION \"%s\"]]]\n", prefix, func);
	receivef("%sFUNCTION \"%s\"", prefix, func);
	printf("*** error %d\n", SCPI_ErrorPop(SCPI_context()));
	/*ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(dm_counters.set_mode, CALLED_ONCE);
	CU_ASSERT_EQUAL(dm_args.set_mode.mode, mode);
	CU_ASSERT_EQUAL(dm_args.set_mode.params_is_null, TRUE);
	CU_ASSERT_EQUAL(dm_multimeter_state.mode, mode); */
}

static void test_function_impl(const char* const func, const scpimm_mode_t mode, void* user_data) {
	(void) user_data;	//	suppress warning

	test_function_impl2("SENSE:", func, mode);
	test_function_impl2("", func, mode);
}

static void test_function() {
	repeat_for_all_modes(test_function_impl, NO_USER_DATA);
}

/*****************************************************************************
 * SENSE:FUNC?
*****************************************************************************/

static void test_functionQ_impl2(const char* const prefix, const char* const func, const scpimm_mode_t mode) {
	char buf[64], fn[64];

	receivef("%sFUNCTION %s", prefix, func);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(dm_multimeter_state.mode, mode);

	strcpy(fn, func);
	shorten_command_name(fn);

	if (strcmp(fn, "VOLT:DC") == 0) {
		strcpy(fn, "VOLT");
	} else if (strcmp(fn, "CURR:DC") == 0) {
		strcpy(fn, "CURR");
	} else if (strcmp(fn, "VOLT:DC:RAT") == 0) {
		strcpy(fn, "VOLT:RAT");
	}

	sprintf(buf, "\"%s\"\r\n", fn);
	receivef("%sFUNCTION?", prefix);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_RESPONSE(buf);
}

static void test_functionQ_impl(const char* const func, const scpimm_mode_t mode, void* user_data) {
	(void) user_data;	//	suppress warning

	test_functionQ_impl2("SENSE:", func, mode);
	test_functionQ_impl2("", func, mode);
}

static void test_functionQ() {
	repeat_for_all_modes(test_functionQ_impl, NO_USER_DATA);
}

/*****************************************************************************
 * SENSE:<function>:RANGE
*****************************************************************************/

static void test_range_value(const char* const prefix, const char* const func, const scpimm_mode_t mode, const char* const value, const size_t expected_range_index) {
	dm_reset_counters();
	dm_reset_args();
	receivef("%s%s:RANGE %s", prefix, func, value);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(dm_counters.set_numeric_param, CALLED_ONCE);
	CU_ASSERT_EQUAL(dm_args.set_numeric_param.mode, mode);
	CU_ASSERT_EQUAL(dm_args.set_numeric_param.param, SCPIMM_PARAM_RANGE);
	CU_ASSERT_EQUAL(dm_args.set_numeric_param.value_index, expected_range_index);
}

static void test_range_value_error(const char* const prefix, const char* const func, const char* const value) {
	dm_reset_counters();
	dm_reset_args();
	receivef("%s%s:RANGE %s", prefix, func, value);
	ASSERT_SCPI_ERROR(SCPI_ERROR_DATA_OUT_OF_RANGE);
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(dm_counters.set_numeric_param, NOT_CALLED);
}

static void test_range_impl2(const char* const prefix, const char* const func, const scpimm_mode_t mode) {
	int16_t err;
	const scpimm_interface_t* const intf = scpimm_interface();
	const double *ranges, *overruns;
	char value[32];
	size_t i;

	CHECK_NO_SCPI_ERROR(intf->get_numeric_param_values(mode, SCPIMM_PARAM_RANGE, &ranges));
	CHECK_NO_SCPI_ERROR(intf->get_numeric_param_values(mode, SCPIMM_PARAM_RANGE_OVERRUN, &overruns));

	test_range_value(prefix, func, mode, "MINimum", min_value_index(ranges));
	test_range_value(prefix, func, mode, "MIN", min_value_index(ranges));
	test_range_value(prefix, func, mode, "MAXimum", max_value_index(ranges));
	test_range_value(prefix, func, mode, "MAX", max_value_index(ranges));

	for (i = 0; ranges[i] >= 0.0; i++) {
		sprintf(value, "%f", ranges[i]);
		test_range_value(prefix, func, mode, value, i);

		sprintf(value, "%f", 0.5 * ranges[i]);
		test_range_value(prefix, func, mode, value, i);

		sprintf(value, "%f", (1.0 - FLOAT_DELTA) * ranges[i] * overruns[i]);
		test_range_value(prefix, func, mode, value, i);
	}

	--i;
	sprintf(value, "%f", 1.1 * ranges[i] * overruns[i]);
	test_range_value_error(prefix, func, value);
}

static void test_range_impl(const char* const func, const scpimm_mode_t mode, void* user_data) {
	if (SCPIMM_MODE_DCV_RATIO == mode || SCPIMM_MODE_ACV_RATIO == mode) {
		return;
	}

	(void) user_data;	//	suppress warning

	test_range_impl2("SENSE:", func, mode);
	test_range_impl2("", func, mode);
}

static void test_range() {
	repeat_for_all_modes(test_range_impl, NO_USER_DATA);
}

/*****************************************************************************
 * SENSE:<function>:RANGE?
*****************************************************************************/

static void test_rangeQ_min_max(const char* const prefix, const char* const func, const char* const suffix, const scpimm_mode_t mode, const char* const value, const double expected_value) {
	dm_reset_counters();
	dm_reset_args();
	receivef("%s%s:%s? %s", prefix, func, suffix, value);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_DOUBLE_RESPONSE(expected_value);
	CU_ASSERT_EQUAL(dm_counters.get_numeric_param_values, CALLED_ONCE);
	CU_ASSERT_EQUAL(dm_args.get_numeric_param_values.mode, mode);
	CU_ASSERT_EQUAL(dm_args.get_numeric_param_values.param, SCPIMM_PARAM_RANGE);
	CU_ASSERT_EQUAL(dm_args.get_numeric_param_values.values_is_null, FALSE);

}

static void test_rangeQ_value(const char* const prefix, const char* const func, const char* const suffix, const scpimm_mode_t mode, const double value, const double expected_value) {
	receivef("%s%s:%s %f", prefix, func, suffix, value);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();

	dm_reset_counters();
	dm_reset_args();
	receivef("%s%s:%s?", prefix, func, suffix);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_DOUBLE_RESPONSE(expected_value);
	CU_ASSERT_EQUAL(dm_counters.get_numeric_param, CALLED_ONCE);
	CU_ASSERT_EQUAL(dm_args.get_numeric_param.mode, mode);
	CU_ASSERT_EQUAL(dm_args.get_numeric_param.param, SCPIMM_PARAM_RANGE);
	CU_ASSERT_EQUAL(dm_args.get_numeric_param.value_is_null, FALSE);
}

static void test_rangeQ_impl2(const char* const prefix, const char* const func, const scpimm_mode_t mode) {
	int16_t err;
	const scpimm_interface_t* const intf = scpimm_interface();
	const double *ranges, *overruns;
	size_t i;

	CHECK_NO_SCPI_ERROR(intf->get_numeric_param_values(mode, SCPIMM_PARAM_RANGE, &ranges));
	CHECK_NO_SCPI_ERROR(intf->get_numeric_param_values(mode, SCPIMM_PARAM_RANGE_OVERRUN, &overruns));

	test_rangeQ_min_max(prefix, func, "RANGE", mode, "MINimum", ranges[min_value_index(ranges)]);
	test_rangeQ_min_max(prefix, func, "RANGE", mode, "MIN", ranges[min_value_index(ranges)]);
	test_rangeQ_min_max(prefix, func, "RANGE", mode, "MAXimum", ranges[max_value_index(ranges)]);
	test_rangeQ_min_max(prefix, func, "RANGE", mode, "MAX", ranges[max_value_index(ranges)]);

	test_rangeQ_min_max(prefix, func, "RANG", mode, "MINimum", ranges[min_value_index(ranges)]);
	test_rangeQ_min_max(prefix, func, "RANG", mode, "MIN", ranges[min_value_index(ranges)]);
	test_rangeQ_min_max(prefix, func, "RANG", mode, "MAXimum", ranges[max_value_index(ranges)]);
	test_rangeQ_min_max(prefix, func, "RANG", mode, "MAX", ranges[max_value_index(ranges)]);

	for (i = 0; ranges[i] >= 0.0; i++) {
		test_rangeQ_value(prefix, func, "RANGE", mode, ranges[i], ranges[i]);
		test_rangeQ_value(prefix, func, "RANG", mode, ranges[i], ranges[i]);
	}
}

static void test_rangeQ_impl(const char* const func, const scpimm_mode_t mode, void* user_data) {
	if (SCPIMM_MODE_DCV_RATIO == mode || SCPIMM_MODE_ACV_RATIO == mode) {
		return;
	}

	(void) user_data;	//	suppress warning

	test_rangeQ_impl2("SENSE:", func, mode);
	test_rangeQ_impl2("", func, mode);
}

static void test_rangeQ() {
	repeat_for_all_modes(test_rangeQ_impl, NO_USER_DATA);
}

/*****************************************************************************
 * SENSE:<function>:RANGE:AUTO
*****************************************************************************/

static void test_range_auto() {
	repeat_for_all_modes(test_set_bool_param_impl, (void*) &RANGE_AUTO_CONTEXT);
}

/*****************************************************************************
 * SENSE:<function>:RANGE:AUTO?
*****************************************************************************/

static void test_range_autoQ() {
	repeat_for_all_modes(test_get_bool_param_impl, (void*) &RANGE_AUTO_CONTEXT);
}

/*****************************************************************************
 * SENSE:<function>:RESOLUTION
*****************************************************************************/

/*****************************************************************************
 * SENSE:<function>:NPLC
*****************************************************************************/

static void test_nplc() {
	repeat_for_all_dc_modes(test_set_numeric_param_impl, (void*) &NPLC_CONTEXT);
}

/*****************************************************************************
 * SENSE:<function>:NPLC?
*****************************************************************************/

static void test_nplcQ() {
	repeat_for_all_dc_modes(test_get_numeric_param_impl, (void*) &NPLC_CONTEXT);
}

/*****************************************************************************
 * SENSE:ZERO:AUTO
*****************************************************************************/

static void test_zero_auto() {
	static const char* const values[] = {"ONCE", "ON", "OFF", "1", "0", NULL};
	static scpi_bool_t const zero_auto[] = {TRUE, TRUE, FALSE, TRUE, FALSE};
	static scpi_bool_t const zero_once[] = {TRUE, FALSE, FALSE, FALSE, FALSE};
	const char* const* prefix;

	for (prefix = SENSE_PREFIXES; *prefix; prefix++) {
		size_t value_index;

		for (value_index = 0; values[value_index]; value_index++) {
			dm_reset_counters();
			dm_reset_args();

			receivef("%sZERO:AUTO %s", *prefix, values[value_index]);
			ASSERT_NO_SCPI_ERRORS();
			ASSERT_NO_RESPONSE();
			CU_ASSERT_EQUAL(dm_counters.set_global_bool_param, CALLED_TWICE);

			CU_ASSERT_EQUAL(dm_prev_args.set_global_bool_param.param, SCPIMM_PARAM_ZERO_AUTO);
			CU_ASSERT_EQUAL(dm_prev_args.set_global_bool_param.value, zero_auto[value_index]);

			CU_ASSERT_EQUAL(dm_args.set_global_bool_param.param, SCPIMM_PARAM_ZERO_AUTO_ONCE);
			CU_ASSERT_EQUAL(dm_args.set_global_bool_param.value, zero_once[value_index]);

			receivef("%sZERO:AUTO?", *prefix);
			ASSERT_NO_SCPI_ERRORS();
			ASSERT_BOOL_RESPONSE(zero_auto[value_index]);
			CU_ASSERT_EQUAL(dm_counters.get_global_bool_param, CALLED_ONCE);
			CU_ASSERT_EQUAL(dm_args.get_global_bool_param.param, SCPIMM_PARAM_ZERO_AUTO);
			CU_ASSERT_EQUAL(dm_args.get_global_bool_param.value_is_null, FALSE);
		}
	}
}

/*****************************************************************************
 * Entry
*****************************************************************************/

int test_sense() {
    CU_pSuite pSuite = NULL;

    ADD_SUITE("SENSE");

    ADD_TEST(test_function);
    /*ADD_TEST(test_functionQ);
    ADD_TEST(test_range);
    ADD_TEST(test_rangeQ);
    ADD_TEST(test_range_auto);
    ADD_TEST(test_range_autoQ);
    ADD_TEST(test_nplc);
    ADD_TEST(test_nplcQ);
    ADD_TEST(test_zero_auto);*/

    return 0;
}

#ifdef	NO_GLOBAL_TEST

int main() {
    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    if (test_sense()) {
    	return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    return RUN_ALL_TESTS();
}

#endif
