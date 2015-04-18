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

static void test_display_impl(const char* cmd, scpi_bool_t expected) {
	receive(cmd);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();

	receive("DISPLAY?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_BOOL_RESPONSE(expected);
}

void test_display() {
	test_display_impl("DISPLAY 0", FALSE);
	test_display_impl("DISPLAY 1", TRUE);
	test_display_impl("DISPLAY +0", FALSE);
	test_display_impl("DISPLAY +1", TRUE);
	test_display_impl("DISPLAY OFF", FALSE);
	test_display_impl("DISPLAY ON", TRUE);
	test_display_impl("DISPLAY Off", FALSE);
	test_display_impl("DISPLAY On", TRUE);
}

void test_displayQ() {
	receive("DISPLAY OFF");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	receive("DISPLAY?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_BOOL_RESPONSE(FALSE);

	receive("DISPLAY ON");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	receive("DISPLAY?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_BOOL_RESPONSE(TRUE);

	receive("DISPLAY OFF");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	receive("DISPLAY?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_BOOL_RESPONSE(FALSE);
}

void test_display_text() {
	dm_reset_counters();
	dm_reset_args();
	receive("DISPLAY:TEXT ''");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
    CU_ASSERT_EQUAL(dm_counters.display_text, CALLED_ONCE);
    CU_ASSERT_STRING_EQUAL(dm_args.display_text.txt, "");
    CU_ASSERT_STRING_EQUAL(dm_display, "");

	dm_reset_counters();
	dm_reset_args();
	receive("DISPLAY:TEXT '1234567890AB'");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
    CU_ASSERT_EQUAL(dm_counters.display_text, CALLED_ONCE);
    CU_ASSERT_STRING_EQUAL(dm_args.display_text.txt, "1234567890AB");
    CU_ASSERT_STRING_EQUAL(dm_display, "1234567890AB");

	receive("DISPLAY:TEXT '1234567890ABC'");
	ASSERT_SCPI_ERROR(SCPIMM_ERROR_TOO_MUCH_DATA);
	ASSERT_NO_RESPONSE();
    CU_ASSERT_EQUAL(dm_counters.display_text, CALLED_ONCE);
    CU_ASSERT_STRING_EQUAL(dm_args.display_text.txt, "1234567890AB");
    CU_ASSERT_STRING_EQUAL(dm_display, "1234567890AB");
}

void test_display_textQ() {
	receive("DISPLAY:TEXT ''");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	receive("DISPLAY:TEXT?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_RESPONSE("\"\"\r\n");

	receive("DISPLAY:TEXT '1234567890AB'");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	receive("DISPLAY:TEXT?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_RESPONSE("\"1234567890AB\"\r\n");
}

void test_display_text_clear() {
	dm_reset_counters();
	dm_reset_args();
	receive("DISPLAY:TEXT '1234567890AB'");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
    CU_ASSERT_EQUAL(dm_counters.display_text, CALLED_ONCE);
    CU_ASSERT_STRING_EQUAL(dm_args.display_text.txt, "1234567890AB");
    CU_ASSERT_STRING_EQUAL(dm_display, "1234567890AB");

	receive("DISPLAY:TEXT:CLEAR");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
    CU_ASSERT_EQUAL(dm_counters.display_text, 2);
    CU_ASSERT_STRING_EQUAL(dm_args.display_text.txt, "");
    CU_ASSERT_STRING_EQUAL(dm_display, "");
}

int test_dispay() {
    CU_pSuite pSuite = NULL;

    ADD_SUITE("DISPLAY");

    /* Add the tests to the suite */
    ADD_TEST(test_display);
    ADD_TEST(test_displayQ);
    ADD_TEST(test_display_text);
    ADD_TEST(test_display_textQ);
    ADD_TEST(test_display_text_clear);

    return 0;
}

int main() {
    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    if (test_dispay()) {
    	return CU_get_error();
    }

    return RUN_ALL_TESTS();
}

