#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "test_utils.h"

static void test_count() {
	scpimm_context_t* const ctx = SCPIMM_context();
	int i;

	ctx->sample_count_num = 0;
	receive("SAMPLE:COUNT MIN");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(ctx->sample_count_num, 1);

	receive("SAMPLE:COUNT MAX");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(ctx->sample_count_num, MAX_SAMPLE_COUNT);

	for (i = 1; i <= MAX_SAMPLE_COUNT; i += 123) {
		receivef("SAMPLE:COUNT %d", i);
		ASSERT_NO_SCPI_ERRORS();
		ASSERT_NO_RESPONSE();
		CU_ASSERT_EQUAL((int) ctx->sample_count_num, i);
	}

	receive("SAMPLE:COUNT 3");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL((int) ctx->sample_count_num, 3);

	receive("SAMPLE:COUNT 0");
	ASSERT_SCPI_ERROR(SCPI_ERROR_DATA_OUT_OF_RANGE);
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL((int) ctx->sample_count_num, 3);

	receivef("SAMPLE:COUNT %d", (int) (MAX_SAMPLE_COUNT + 1));
	ASSERT_SCPI_ERROR(SCPI_ERROR_DATA_OUT_OF_RANGE);
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL((int) ctx->sample_count_num, 3);

	receive("SAMPLE:COUNT DEF");
	ASSERT_SCPI_ERROR(SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL((int) ctx->sample_count_num, 3);
}

static void test_countQ() {
	scpimm_context_t* const ctx = SCPIMM_context();
	int i;

	receive("SAMPLE:COUNT? MIN");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_INT_RESPONSE(1);

	receive("SAMPLE:COUNT? MAX");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_INT_RESPONSE(MAX_SAMPLE_COUNT);

	for (i = 1; i <= MAX_SAMPLE_COUNT; i += 123) {
		ctx->sample_count_num = i;
		receive("SAMPLE:COUNT?");
		ASSERT_NO_SCPI_ERRORS();
		ASSERT_INT_RESPONSE(i);
	}
}

int test_sample() {
    CU_pSuite pSuite = NULL;

    /* Add a suite to the registry */
    ADD_SUITE("SAMPLE");

    /* Add the tests to the suite */
    ADD_TEST(test_count);
    ADD_TEST(test_countQ);

    return 0;
}

#ifdef	NO_GLOBAL_TEST

int main() {
    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    if (test_sample()) {
    	return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    return RUN_ALL_TESTS();
}

#endif
