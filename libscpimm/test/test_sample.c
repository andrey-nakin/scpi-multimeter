#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "test_utils.h"

int init_suite(void) {
    return 0;
}

int clean_suite(void) {
    return 0;
}

void test_count() {
	scpimm_context_t* const ctx = SCPIMM_context();
	int i;

	init_scpimm();

	ctx->sample_count_num = 0;
	receive("SAMPLE:COUNT MIN");
	assert_no_scpi_errors();
	assert_no_data();
	CU_ASSERT_EQUAL(ctx->sample_count_num, 1);

	receive("SAMPLE:COUNT MAX");
	assert_no_scpi_errors();
	assert_no_data();
	CU_ASSERT_EQUAL(ctx->sample_count_num, SCPIMM_BUF_CAPACITY);

	for (i = 1; i <= SCPIMM_BUF_CAPACITY; ++i) {
		receivef("SAMPLE:COUNT %d", i);
		assert_no_scpi_errors();
		assert_no_data();
		CU_ASSERT_EQUAL((int) ctx->sample_count_num, i);
	}

	receive("SAMPLE:COUNT 0");
	assert_scpi_error(SCPI_ERROR_DATA_OUT_OF_RANGE);
	assert_no_data();

	receivef("SAMPLE:COUNT %d", (int) (SCPIMM_BUF_CAPACITY + 1));
	assert_scpi_error(SCPI_ERROR_DATA_OUT_OF_RANGE);
	assert_no_data();

	receive("SAMPLE:COUNT DEF");
	assert_scpi_error(SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	assert_no_data();
}

void test_countQ() {
	scpimm_context_t* const ctx = SCPIMM_context();
	int i;

	init_scpimm();

	receive("SAMPLE:COUNT? MIN");
	assert_no_scpi_errors();
	assert_in_int(1);

	receive("SAMPLE:COUNT? MAX");
	assert_no_scpi_errors();
	assert_in_int(SCPIMM_BUF_CAPACITY);

	for (i = 1; i <= SCPIMM_BUF_CAPACITY; ++i) {
		ctx->sample_count_num = i;
		receive("SAMPLE:COUNT?");
		assert_no_scpi_errors();
		assert_in_int(i);
	}
}

int main() {
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    pSuite = CU_add_suite("SAMPLE", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "test sample:count", test_count))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test sample:count?", test_countQ))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

