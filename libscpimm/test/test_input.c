#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "test_utils.h"

static void test_impedance_auto() {
	const scpimm_context_t* const ctx = SCPIMM_context();

	dm_reset_counters();
	dm_reset_args();
	receive("INPUT:IMPEDANCE:AUTO ON");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(dm_counters.set_global_bool_option, CALLED_ONCE);
	CU_ASSERT_EQUAL(dm_set_global_bool_option_args.option, SCPIMM_OPTION_INPUT_IMPEDANCE_AUTO);
	CU_ASSERT_EQUAL(dm_set_global_bool_option_args.value, TRUE);
	CU_ASSERT_EQUAL(ctx->input_impedance_auto_state, TRUE);
	
	dm_reset_counters();
	dm_reset_args();
	receive("INPUT:IMPEDANCE:AUTO OFF");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(dm_counters.set_global_bool_option, CALLED_ONCE);
	CU_ASSERT_EQUAL(dm_set_global_bool_option_args.option, SCPIMM_OPTION_INPUT_IMPEDANCE_AUTO);
	CU_ASSERT_EQUAL(dm_set_global_bool_option_args.value, FALSE);
	CU_ASSERT_EQUAL(ctx->input_impedance_auto_state, FALSE);

	dm_reset_counters();
	receive("INPUT:IMPEDANCE:AUTO");
	ASSERT_SCPI_ERROR(SCPI_ERROR_MISSING_PARAMETER);
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(dm_counters.set_global_bool_option, NOT_CALLED);
	CU_ASSERT_EQUAL(ctx->input_impedance_auto_state, FALSE);

	// emulate error
	dm_returns.set_global_bool_option = SCPI_ERROR_UNKNOWN;
	dm_reset_counters();
	dm_reset_args();
	receive("INPUT:IMPEDANCE:AUTO ON");
	ASSERT_SCPI_ERROR(SCPI_ERROR_UNKNOWN);
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(dm_counters.set_global_bool_option, CALLED_ONCE);
	CU_ASSERT_EQUAL(dm_set_global_bool_option_args.option, SCPIMM_OPTION_INPUT_IMPEDANCE_AUTO);
	CU_ASSERT_EQUAL(dm_set_global_bool_option_args.value, TRUE);
	CU_ASSERT_EQUAL(ctx->input_impedance_auto_state, FALSE);

	// reset error
	dm_returns.set_global_bool_option = SCPI_ERROR_OK;
}

static void test_impedance_autoQ() {
	scpimm_context_t* const ctx = SCPIMM_context();

	ctx->input_impedance_auto_state = TRUE;
	receive("INPUT:IMPEDANCE:AUTO?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_BOOL_RESPONSE(TRUE);

	ctx->input_impedance_auto_state = FALSE;
	receive("INPUT:IMPEDANCE:AUTO?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_BOOL_RESPONSE(FALSE);
}

int main() {
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    ADD_SUITE("INPUT");

    /* Add the tests to the suite */
    ADD_TEST(test_impedance_auto);
    ADD_TEST(test_impedance_autoQ);

    /* Run all tests using the CUnit Basic interface */
    return RUN_ALL_TESTS();
}

