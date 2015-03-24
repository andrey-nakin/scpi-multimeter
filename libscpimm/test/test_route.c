#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "test_utils.h"

void test_terminalsQ() {
	scpimm_interface_t* const intf = SCPIMM_context()->interface;

	receive("*RST");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();

	receive("ROUTE:TERMINALS?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_RESPONSE("FRON\r\n");

	dm_multimeter_state.terminal_state = SCPIMM_TERM_REAR;
	receive("ROUTE:TERMINALS?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_RESPONSE("REAR\r\n");

	// emulate error
	dm_returns.get_input_terminal = SCPI_ERROR_UNKNOWN;
	receive("ROUTE:TERMINALS?");
	ASSERT_SCPI_ERROR(SCPI_ERROR_UNKNOWN);
	dm_returns.get_input_terminal = SCPI_ERROR_OK;

	// emulate "handler not specified" case
	intf->get_input_terminal = NULL;
	receive("ROUTE:TERMINALS?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_RESPONSE("FRON\r\n");
}

int main() {
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    ADD_SUITE("ROUTE");

    /* Add the tests to the suite */
    ADD_TEST(test_terminalsQ);

    /* Run all tests using the CUnit Basic interface */
    return RUN_ALL_TESTS();
}
