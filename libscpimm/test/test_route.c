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

static void test_terminalsQ() {
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
	dm_returns.get_input_terminal = SCPIMM_ERROR_INTERNAL;
	receive("ROUTE:TERMINALS?");
	ASSERT_SCPI_ERROR(SCPIMM_ERROR_INTERNAL);
	dm_returns.get_input_terminal = SCPIMM_ERROR_OK;

	// emulate "handler not specified" case
	intf->get_input_terminal = NULL;
	receive("ROUTE:TERMINALS?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_RESPONSE("FRON\r\n");
}

int test_route() {
    CU_pSuite pSuite = NULL;

    /* Add a suite to the registry */
    ADD_SUITE("ROUTE");

    /* Add the tests to the suite */
    ADD_TEST(test_terminalsQ);

    return 0;
}

#ifdef	NO_GLOBAL_TEST

int main() {
    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();


    if (test_route()) {
    	return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    return RUN_ALL_TESTS();
}

#endif
