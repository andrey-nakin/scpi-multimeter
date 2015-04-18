/*
    Copyright (c) 2015 Andrey Nakin
    All Rights Reserved

	This file is part of scpi-multimeter library.

	v7-28-arduino is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	v7-28-arduino is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with v7-28-arduino.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "test_utils.h"

static void test_impedance_auto() {
	dm_reset_counters();
	dm_reset_args();
	receive("INPUT:IMPEDANCE:AUTO ON");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(dm_counters.set_global_bool_param, CALLED_ONCE);
	CU_ASSERT_EQUAL(dm_args.set_global_bool_param.param, SCPIMM_PARAM_INPUT_IMPEDANCE_AUTO);
	CU_ASSERT_EQUAL(dm_args.set_global_bool_param.value, TRUE);
	CU_ASSERT_EQUAL(dm_multimeter_state.input_impedance_auto_state, TRUE);
	
	dm_reset_counters();
	dm_reset_args();
	receive("INPUT:IMPEDANCE:AUTO OFF");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(dm_counters.set_global_bool_param, CALLED_ONCE);
	CU_ASSERT_EQUAL(dm_args.set_global_bool_param.param, SCPIMM_PARAM_INPUT_IMPEDANCE_AUTO);
	CU_ASSERT_EQUAL(dm_args.set_global_bool_param.value, FALSE);
	CU_ASSERT_EQUAL(dm_multimeter_state.input_impedance_auto_state, FALSE);

	dm_reset_counters();
	receive("INPUT:IMPEDANCE:AUTO");
	ASSERT_SCPI_ERROR(SCPIMM_ERROR_MISSING_PARAMETER);
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(dm_counters.set_global_bool_param, NOT_CALLED);
	CU_ASSERT_EQUAL(dm_multimeter_state.input_impedance_auto_state, FALSE);

	// emulate error
	dm_returns.set_global_bool_param = SCPIMM_ERROR_INTERNAL;
	dm_reset_counters();
	dm_reset_args();
	receive("INPUT:IMPEDANCE:AUTO ON");
	ASSERT_SCPI_ERROR(SCPIMM_ERROR_INTERNAL);
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(dm_counters.set_global_bool_param, CALLED_ONCE);
	CU_ASSERT_EQUAL(dm_args.set_global_bool_param.param, SCPIMM_PARAM_INPUT_IMPEDANCE_AUTO);
	CU_ASSERT_EQUAL(dm_args.set_global_bool_param.value, TRUE);
	CU_ASSERT_EQUAL(dm_multimeter_state.input_impedance_auto_state, FALSE);

	// reset error
	dm_returns.set_global_bool_param = SCPIMM_ERROR_OK;
}

static void test_impedance_autoQ() {
	dm_multimeter_state.input_impedance_auto_state = TRUE;
	receive("INPUT:IMPEDANCE:AUTO?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_BOOL_RESPONSE(TRUE);

	dm_multimeter_state.input_impedance_auto_state = FALSE;
	receive("INPUT:IMPEDANCE:AUTO?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_BOOL_RESPONSE(FALSE);
}

int test_input() {
    CU_pSuite pSuite = NULL;

    /* Add a suite to the registry */
    ADD_SUITE("INPUT");

    /* Add the tests to the suite */
    ADD_TEST(test_impedance_auto);
    ADD_TEST(test_impedance_autoQ);

    return 0;
}

#ifdef	NO_GLOBAL_TEST

int main() {
    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    if (test_input()) {
    	return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    return RUN_ALL_TESTS();
}

#endif
