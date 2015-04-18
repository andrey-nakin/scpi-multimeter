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

static void test_opcQ() {
	const scpimm_context_t* const ctx = SCPIMM_context();

	receive("*RST;*OPC?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_BOOL_RESPONSE(TRUE);

	receive("CONF:VOLT;:INIT");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT(SCPIMM_STATE_IDLE != ctx->state);

	receive("*OPC?");
	CU_ASSERT(SCPIMM_STATE_IDLE == ctx->state);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_BOOL_RESPONSE(TRUE);

	receive("*RST");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
}

static void test_test() {
	dm_reset_counters();
	dm_reset_args();
	receive("*TST?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_BOOL_RESPONSE(FALSE);
	CU_ASSERT_EQUAL(dm_counters.test, CALLED_ONCE);

	dm_returns.test = SCPIMM_ERROR_INTERNAL;
	dm_reset_counters();
	dm_reset_args();
	receive("*TST?");
	ASSERT_BOOL_RESPONSE(TRUE);
	ASSERT_SCPI_ERROR(SCPIMM_ERROR_INTERNAL);
	CU_ASSERT_EQUAL(dm_counters.test, CALLED_ONCE);

	dm_returns.test = SCPIMM_ERROR_OK;
}

int test_ieee488() {
    CU_pSuite pSuite = NULL;

    ADD_SUITE("IEEE-488");

    /* Add the tests to the suite */
    ADD_TEST(test_opcQ);
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
