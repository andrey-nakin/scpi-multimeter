#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "test_utils.h"

static unsigned counter;
static bool_t last_state;
static bool_t result = TRUE;

static bool_t set_input_impedance_auto(bool_t state) {
	++counter;
	last_state = state;
	return result;
}


int init_suite(void) {
    return 0;
}

int clean_suite(void) {
    return 0;
}

void test_impedance_auto() {
	init_scpimm();

	scpimm_context_t* const ctx = SCPIMM_context();
	scpimm_interface_t* const intf = ctx->interface;

	intf->set_input_impedance_auto = NULL;

	receive("INPUT:IMPEDANCE:AUTO ON");
	assert_no_scpi_errors();
	assert_no_data();
	CU_ASSERT_EQUAL(ctx->input_impedance_auto_state, TRUE);
	
	receive("INPUT:IMPEDANCE:AUTO OFF");
	assert_no_scpi_errors();
	assert_no_data();
	CU_ASSERT_EQUAL(ctx->input_impedance_auto_state, FALSE);

	receive("INPUT:IMPEDANCE:AUTO");
	assert_scpi_error(SCPI_ERROR_MISSING_PARAMETER);
	assert_no_data();

	intf->set_input_impedance_auto = set_input_impedance_auto;
	result = TRUE;

	counter = 0;
	last_state = FALSE;
	receive("INPUT:IMPEDANCE:AUTO ON");
	assert_no_scpi_errors();
	assert_no_data();
	CU_ASSERT_EQUAL(ctx->input_impedance_auto_state, TRUE);
	CU_ASSERT_EQUAL(counter, 1);
	CU_ASSERT_EQUAL(last_state, TRUE);

	receive("INPUT:IMPEDANCE:AUTO OFF");
	assert_no_scpi_errors();
	assert_no_data();
	CU_ASSERT_EQUAL(ctx->input_impedance_auto_state, FALSE);
	CU_ASSERT_EQUAL(counter, 2);
	CU_ASSERT_EQUAL(last_state, FALSE);

	result = FALSE;
	receive("INPUT:IMPEDANCE:AUTO ON");
	assert_scpi_error(SCPI_ERROR_UNKNOWN);
	assert_no_data();
}

void test_impedance_autoQ() {
	init_scpimm();
	scpimm_context_t* const ctx = SCPIMM_context();

	ctx->input_impedance_auto_state = TRUE;
	receive("INPUT:IMPEDANCE:AUTO?");
	assert_no_scpi_errors();
	assert_in_bool(TRUE);

	ctx->input_impedance_auto_state = FALSE;
	receive("INPUT:IMPEDANCE:AUTO?");
	assert_no_scpi_errors();
	assert_in_bool(FALSE);
}

int main() {
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    pSuite = CU_add_suite("INPUT", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "test input:impedance:auto", test_impedance_auto))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test input:impedance:auto?", test_impedance_autoQ))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

