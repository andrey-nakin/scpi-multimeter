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

static void test_count() {
	scpimm_context_t* const ctx = SCPIMM_context();
	int i;

	ctx->sample_count_num = 0;
	receive("SAMPLE:COUNT MIN");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(ctx->sample_count_num, 1);

	receive("SAMPLE:COUNT MAX");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(ctx->sample_count_num, MAX_SAMPLE_COUNT);

	for (i = 1; i <= MAX_SAMPLE_COUNT; i += 123) {
		receivef("SAMPLE:COUNT %d", i);
		ASSERT_NO_SCPI_ERRORS();
		ASSERT_NO_RESPONSE();
		CU_ASSERT_EQUAL((int) ctx->sample_count_num, i);
	}

	receive("SAMPLE:COUNT 3");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL((int) ctx->sample_count_num, 3);

	receive("SAMPLE:COUNT 0");
	ASSERT_SCPI_ERROR(SCPI_ERROR_DATA_OUT_OF_RANGE);
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL((int) ctx->sample_count_num, 3);

	receivef("SAMPLE:COUNT %d", (int) (MAX_SAMPLE_COUNT + 1));
	ASSERT_SCPI_ERROR(SCPI_ERROR_DATA_OUT_OF_RANGE);
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL((int) ctx->sample_count_num, 3);

	receive("SAMPLE:COUNT DEF");
	ASSERT_SCPI_ERROR(SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL((int) ctx->sample_count_num, 3);
}

static void test_countQ() {
	scpimm_context_t* const ctx = SCPIMM_context();
	int i;

	receive("SAMPLE:COUNT? MIN");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_INT_RESPONSE(1);

	receive("SAMPLE:COUNT? MAX");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_INT_RESPONSE(MAX_SAMPLE_COUNT);

	for (i = 1; i <= MAX_SAMPLE_COUNT; i += 123) {
		ctx->sample_count_num = i;
		receive("SAMPLE:COUNT?");
		ASSERT_NO_SCPI_ERRORS();
		ASSERT_INT_RESPONSE(i);
	}
}

int test_sample() {
    CU_pSuite pSuite = NULL;

    /* Add a suite to the registry */
    ADD_SUITE("SAMPLE");

    /* Add the tests to the suite */
    ADD_TEST(test_count);
    ADD_TEST(test_countQ);

    return 0;
}

#ifdef	NO_GLOBAL_TEST

int main() {
    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    if (test_sample()) {
    	return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    return RUN_ALL_TESTS();
}

#endif
