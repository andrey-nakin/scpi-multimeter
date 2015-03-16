#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "test_utils.h"

static scpimm_terminal_state_t terminal;
static bool_t terminal_result;

static bool_t get_input_terminal(scpimm_terminal_state_t* term) {
	*term = terminal;
	return terminal_result;
}

int init_suite(void) {
    return 0;
}

int clean_suite(void) {
    return 0;
}

void test_terminalsQ() {
	init_scpimm();

	scpimm_interface_t* intf = SCPIMM_context()->interface;

	intf->get_input_terminal = NULL;
	receive("ROUTE:TERMINALS?");
	assert_no_scpi_errors();
	assert_in_data("FRON\r\n");

	intf->get_input_terminal = get_input_terminal;
	terminal = SCPIMM_TERM_FRONT;
	terminal_result = TRUE;
	receive("ROUTE:TERMINALS?");
	assert_no_scpi_errors();
	assert_in_data("FRON\r\n");

	terminal = SCPIMM_TERM_REAR;
	receive("ROUTE:TERMINALS?");
	assert_no_scpi_errors();
	assert_in_data("REAR\r\n");

	terminal_result = FALSE;
	receive("ROUTE:TERMINALS?");
	assert_scpi_error(SCPI_ERROR_UNKNOWN);
}

int main() {
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    pSuite = CU_add_suite("ROUTE", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "test route:terminals?", test_terminalsQ))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

