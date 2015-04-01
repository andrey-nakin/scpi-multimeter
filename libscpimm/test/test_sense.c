#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "test_utils.h"

static void test_function_impl2(const char* const prefix, const char* const func, const scpimm_mode_t mode) {
	dm_reset_counters();
	dm_reset_args();

	receivef("%sFUNCTION %s", prefix, func);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(dm_counters.set_mode, CALLED_ONCE);
	CU_ASSERT_EQUAL(dm_args.set_mode.mode, mode);
	CU_ASSERT_EQUAL(dm_args.set_mode.params_is_null, TRUE);
	CU_ASSERT_EQUAL(dm_multimeter_state.mode, mode);
}

static void test_function_impl(const char* const func, const scpimm_mode_t mode, void* user_data) {
	(void) user_data;	//	suppress warning

	test_function_impl2("SENSE:", func, mode);
	test_function_impl2("", func, mode);
}

static void test_function() {
	repeat_for_all_modes(test_function_impl, NO_USER_DATA);
}

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
	if (SCPIMM_MODE_DCV_RATIO == mode) {
		return;
	}

	(void) user_data;	//	suppress warning

	test_range_impl2("SENSE:", func, mode);
	test_range_impl2("", func, mode);
}

static void test_range() {
	repeat_for_all_modes(test_range_impl, NO_USER_DATA);
}

static void test_rangeQ_impl2(const char* const prefix, const char* const func, const scpimm_mode_t mode) {
	int16_t err;
	const scpimm_interface_t* const intf = scpimm_interface();
	const double *ranges, *overruns;
	char value[32];
	size_t i;

	CHECK_NO_SCPI_ERROR(intf->get_numeric_param_values(mode, SCPIMM_PARAM_RANGE, &ranges));
	CHECK_NO_SCPI_ERROR(intf->get_numeric_param_values(mode, SCPIMM_PARAM_RANGE_OVERRUN, &overruns));

	for (i = 0; ranges[i] >= 0.0; i++) {
		sprintf(value, "%f", ranges[i]);
		//test_rangeQ_value(prefix, func, mode, value, ranges[i]);
	}
}

static void test_rangeQ_impl(const char* const func, const scpimm_mode_t mode, void* user_data) {
	if (SCPIMM_MODE_DCV_RATIO == mode) {
		return;
	}

	(void) user_data;	//	suppress warning

	test_rangeQ_impl2("SENSE:", func, mode);
	test_rangeQ_impl2("", func, mode);
}

static void test_rangeQ() {
	repeat_for_all_modes(test_rangeQ_impl, NO_USER_DATA);
}

int test_sense() {
    CU_pSuite pSuite = NULL;

    ADD_SUITE("SENSE");

    ADD_TEST(test_function);
    ADD_TEST(test_functionQ);
    ADD_TEST(test_range);
    ADD_TEST(test_rangeQ);

    return 0;
}

int main() {
    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    test_sense();

    /* Run all tests using the CUnit Basic interface */
    return RUN_ALL_TESTS();
}

