#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "test_utils.h"

#define FLOAT_DELTA 1.0e-6
#define CONTEXT (SCPIMM_context())

static void delay_auto_impl(const char* cmd, bool_t expected) {
	receive(cmd);
	assert_no_scpi_errors();
	assert_no_data();
    CU_ASSERT_EQUAL(CONTEXT->trigger_auto_delay, expected);
}

int init_suite(void) {
	init_scpimm();
    return 0;
}

int clean_suite(void) {
    return 0;
}

void test_source() {
	scpimm_context_t* const ctx = SCPIMM_context();
	const char* options[] = {"BUS", "IMM", "EXT"};
	const scpimm_trig_src_t expected[] = {SCPIMM_TRIG_BUS, SCPIMM_TRIG_IMM, SCPIMM_TRIG_EXT};
	size_t i;

	ctx->trigger_src = (scpimm_trig_src_t) -1;
	for (i = 0; i < sizeof(options) / sizeof(options[0]); ++i) {
		receivef("TRIGGER:SOURCE %s", options[i]);
		assert_no_scpi_errors();
		assert_no_data();
		CU_ASSERT_EQUAL(ctx->trigger_src, expected[i]);
	}

	receive("TRIGGER:SOURCE WRONG");
	assert_scpi_error(SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	assert_no_data();
}

void test_sourceQ() {
	scpimm_context_t* const ctx = SCPIMM_context();
	const scpimm_trig_src_t values[] = {SCPIMM_TRIG_BUS, SCPIMM_TRIG_IMM, SCPIMM_TRIG_EXT};
	const char* expected[] = {"BUS\r\n", "IMM\r\n", "EXT\r\n"};
	size_t i;

	init_scpimm();

	for (i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
		ctx->trigger_src = values[i];
		receive("TRIGGER:SOURCE?");
		assert_no_scpi_errors();
		assert_in_data(expected[i]);
	}
}

void test_delay() {
	scpimm_context_t* const ctx = SCPIMM_context();
	float f;

	init_scpimm();

	ctx->trigger_count_num = 0;
	ctx->infinite_trigger_count = TRUE;

	receive("TRIGGER:DELAY MIN");
	assert_no_scpi_errors();
	assert_no_data();
	CU_ASSERT_DOUBLE_EQUAL(ctx->trigger_delay, 0.0f, FLOAT_DELTA);

	receive("TRIGGER:DELAY MAX");
	assert_no_scpi_errors();
	assert_no_data();
	CU_ASSERT_DOUBLE_EQUAL(ctx->trigger_delay, 3600.0f, FLOAT_DELTA);

	for (f = 0.0f; f <= 3600.0f; f += 123.45f) {
		receivef("TRIGGER:DELAY %f", (double) f);
		assert_no_scpi_errors();
		assert_no_data();
		CU_ASSERT_DOUBLE_EQUAL(ctx->trigger_delay, f, FLOAT_DELTA);
	}

	receive("TRIGGER:DELAY -1");
	assert_scpi_error(SCPI_ERROR_DATA_OUT_OF_RANGE);
	assert_no_data();

	receive("TRIGGER:DELAY 3601");
	assert_scpi_error(SCPI_ERROR_DATA_OUT_OF_RANGE);
	assert_no_data();

	receive("TRIGGER:COUNT DEF");
	assert_scpi_error(SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	assert_no_data();
}

void test_delayQ() {
	scpimm_context_t* const ctx = SCPIMM_context();
	double f;

	init_scpimm();

	ctx->trigger_delay = 123.45;

	receive("TRIGGER:DELAY? MIN");
	assert_no_scpi_errors();
	assert_in_double(0);

	receive("TRIGGER:DELAY? MAX");
	assert_no_scpi_errors();
	assert_in_double(3600);

	ctx->infinite_trigger_count = FALSE;
	for (f = 0; f <= 3600; f += 123.45) {
		ctx->trigger_delay = f;
		receive("TRIGGER:DELAY?");
		assert_no_scpi_errors();
		assert_in_double(f);
	}
}

void test_delay_auto() {
	delay_auto_impl("TRIGGER:DELAY:AUTO 0", FALSE);
	delay_auto_impl("TRIGGER:DELAY:AUTO 1", TRUE);
	delay_auto_impl("TRIGGER:DELAY:AUTO +0", FALSE);
	delay_auto_impl("TRIGGER:DELAY:AUTO +1", TRUE);
	delay_auto_impl("TRIGGER:DELAY:AUTO OFF", FALSE);
	delay_auto_impl("TRIGGER:DELAY:AUTO ON", TRUE);
	delay_auto_impl("TRIGGER:DELAY:AUTO Off", FALSE);
	delay_auto_impl("TRIGGER:DELAY:AUTO On", TRUE);
}

void test_delay_autoQ() {
	CONTEXT->trigger_auto_delay = FALSE;
	receive("TRIGGER:DELAY:AUTO?");
	assert_no_scpi_errors();
	assert_in_bool(FALSE);

	CONTEXT->trigger_auto_delay = TRUE;
	receive("TRIGGER:DELAY:AUTO?");
	assert_no_scpi_errors();
	assert_in_bool(TRUE);
}

void test_count() {
	scpimm_context_t* const ctx = SCPIMM_context();
	int i;

	init_scpimm();

	ctx->trigger_count_num = 0;
	ctx->infinite_trigger_count = TRUE;

	receive("TRIGGER:COUNT MIN");
	assert_no_scpi_errors();
	assert_no_data();
	CU_ASSERT_EQUAL(ctx->trigger_count_num, 1);
	CU_ASSERT_EQUAL(ctx->infinite_trigger_count, FALSE);

	receive("TRIGGER:COUNT INF");
	assert_no_scpi_errors();
	assert_no_data();
	CU_ASSERT_EQUAL(ctx->infinite_trigger_count, TRUE);

	receive("TRIGGER:COUNT MAX");
	assert_no_scpi_errors();
	assert_no_data();
	CU_ASSERT_EQUAL(ctx->trigger_count_num, MAX_TRIGGER_COUNT);
	CU_ASSERT_EQUAL(ctx->infinite_trigger_count, FALSE);

	for (i = 1; i <= SCPIMM_BUF_CAPACITY; ++i) {
		receivef("TRIGGER:COUNT %d", i);
		assert_no_scpi_errors();
		assert_no_data();
		CU_ASSERT_EQUAL((int) ctx->trigger_count_num, i);
		CU_ASSERT_EQUAL(ctx->infinite_trigger_count, FALSE);
	}

	receive("TRIGGER:COUNT 0");
	assert_scpi_error(SCPI_ERROR_DATA_OUT_OF_RANGE);
	assert_no_data();

	receivef("TRIGGER:COUNT %d", (int) (SCPIMM_BUF_CAPACITY + 1));
	assert_scpi_error(SCPI_ERROR_DATA_OUT_OF_RANGE);
	assert_no_data();

	receive("TRIGGER:COUNT DEF");
	assert_scpi_error(SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	assert_no_data();
}

void test_countQ() {
	scpimm_context_t* const ctx = SCPIMM_context();
	int i;

	init_scpimm();

	receive("TRIGGER:COUNT? MIN");
	assert_no_scpi_errors();
	assert_in_int(1);

	receive("TRIGGER:COUNT? MAX");
	assert_no_scpi_errors();
	assert_in_int(SCPIMM_BUF_CAPACITY);

	ctx->infinite_trigger_count = FALSE;
	for (i = 1; i <= SCPIMM_BUF_CAPACITY; ++i) {
		ctx->trigger_count_num = i;
		receive("TRIGGER:COUNT?");
		assert_no_scpi_errors();
		assert_in_int(i);
	}

	ctx->infinite_trigger_count = TRUE;
	receive("TRIGGER:COUNT?");
	assert_no_scpi_errors();
	assert_in_double(9.90000000E+37);
}

int main() {
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    pSuite = CU_add_suite("TRIGGER", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "trigger:source", test_source))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "trigger:source?", test_sourceQ))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "trigger:delay", test_delay))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "trigger:delay?", test_delayQ))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "trigger:delay:auto", test_delay_auto))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "trigger:delay:auto?", test_delay_autoQ))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "trigger:count", test_count))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "trigger:count?", test_countQ))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

