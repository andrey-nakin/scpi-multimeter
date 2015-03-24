#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "test_utils.h"

void test_functionQ() {
	receive("SENSE:FUNCTION?");
	ASSERT_NO_SCPI_ERRORS();
	//assert_in_data("v1.0\r\n");
}

int main() {
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    ADD_SUITE("SENSE");

    /* Add the tests to the suite */
    ADD_TEST(test_functionQ);

    /* Run all tests using the CUnit Basic interface */
    return RUN_ALL_TESTS();
}

