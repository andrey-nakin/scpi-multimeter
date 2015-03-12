#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "test_utils.h"
#include "default_multimeter.h"

static void reset() {
	init_scpimm();
	clearscpi_errors();
	init_test_vars();
}

int init_suite(void) {
    return 0;
}

int clean_suite(void) {
    return 0;
}

void test_readQ_generic_impl(const dm_measurement_type_t mt) {
	char *result = dm_output_buffer();
	double actual_range, actual_resolution, value;

	reset();
	dm_multimeter_config.measurement_type = mt;

	receivef("CONFIGURE?");
	assert_no_scpi_errors();
	sscanf(strchr(result, ' ') + 1, "%le,%le", &actual_range, &actual_resolution);

	dm_reset_counters();
	receivef("READ?");
	assert_no_scpi_errors();
	sscanf(result, "%le", &value);
	CU_ASSERT_EQUAL(dm_counters.start_measure, 1);
	ASSERT_DOUBLE_EQUAL(value, actual_range * 0.5);
}

void test_readQ_generic() {
	test_readQ_generic_impl(DM_MEASUREMENT_TYPE_ASYNC);
	test_readQ_generic_impl(DM_MEASUREMENT_TYPE_SYNC);
}

void test_readQ() {
//	test_readQ_generic();
}

int main() {
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    pSuite = CU_add_suite("DMM", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "read? generic", test_readQ_generic))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

