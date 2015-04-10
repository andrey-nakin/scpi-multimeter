#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "test_utils.h"

static void test_test() {
	dm_reset_counters();
	dm_reset_args();
	receive("*TST?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_BOOL_RESPONSE(FALSE);
	CU_ASSERT_EQUAL(dm_counters.test, CALLED_ONCE);

	dm_returns.test = SCPI_ERROR_UNKNOWN;
	dm_reset_counters();
	dm_reset_args();
	receive("*TST?");
	ASSERT_BOOL_RESPONSE(TRUE);
	ASSERT_SCPI_ERROR(SCPI_ERROR_UNKNOWN);
	CU_ASSERT_EQUAL(dm_counters.test, CALLED_ONCE);

	dm_returns.test = SCPI_ERROR_OK;
}

int test_ieee488() {
    CU_pSuite pSuite = NULL;

    ADD_SUITE("IEEE-488");

    /* Add the tests to the suite */
    ADD_TEST(test_test);

    return 0;
}

#ifdef	NO_GLOBAL_TEST

int main() {

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    if (test_ieee488()) {
    	return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    return RUN_ALL_TESTS();
}

#endif
