#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>

#define CONTEXT (SCPIMM_context())

static int mode_counter;
static uint16_t last_mode;
static float last_range;
static float last_resolution;

static bool_t set_mode(const uint16_t mode, float range, float resolution) {
	++mode_counter;
	last_mode = mode;
	last_range = range;
	last_resolution = resolution;

	return TRUE;
}

static void reset() {
	mode_counter = 0;
	last_mode = 0;
	last_range = -100;
	last_resolution = -100;

	init_scpimm();
	clearscpi_errors();
	init_test_vars();
	SCPIMM_context()->interface->set_mode = set_mode;
}

static void check_last_mode(uint16_t mode, float range, float resolution) {
	const scpimm_context_t* ctx = SCPIMM_context();

    CU_ASSERT_EQUAL(mode_counter, 1);
    CU_ASSERT_EQUAL(last_mode, mode);
    CU_ASSERT_EQUAL(ctx->mode, mode);
    CU_ASSERT_EQUAL(last_range, range);
    CU_ASSERT_EQUAL(last_resolution, resolution);
}

/* configure function with default range and resolution */
static void test_configure_no_params(const char* function, uint16_t mode) {
	receivef("CONFIGURE:%s\r\n", function);
	assert_no_scpi_errors();
	asset_no_data();
	check_last_mode(mode, SCPIMM_RANGE_DEF, SCPIMM_RESOLUTION_DEF);
}

static void test_impl(const char* function, uint16_t mode) {
	reset();
	test_configure_no_params(function, mode);
}

int init_suite(void) {
    return 0;
}

int clean_suite(void) {
    return 0;
}

void test_configure_voltage_dc() {
	test_impl("VOLTAGE:DC", SCPIMM_MODE_DCV);
}


int main() {
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    pSuite = CU_add_suite("CONFIGURE", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "test configure:voltage:dc", test_configure_voltage_dc))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

