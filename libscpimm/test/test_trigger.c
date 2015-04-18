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

#define CONTEXT (SCPIMM_context())

static void delay_auto_impl(const char* cmd, scpi_bool_t expected) {
	receive(cmd);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
    CU_ASSERT_EQUAL(CONTEXT->trigger_auto_delay, expected);
}

static void test_source() {
	scpimm_context_t* const ctx = SCPIMM_context();
	const char* options[] = {"BUS", "IMM", "EXT"};
	const scpimm_trig_src_t expected[] = {SCPIMM_TRIG_BUS, SCPIMM_TRIG_IMM, SCPIMM_TRIG_EXT};
	size_t i;

	ctx->trigger_src = (scpimm_trig_src_t) -1;
	for (i = 0; i < sizeof(options) / sizeof(options[0]); ++i) {
		receivef("TRIGGER:SOURCE %s", options[i]);
		ASSERT_NO_SCPI_ERRORS();
		ASSERT_NO_RESPONSE();
		CU_ASSERT_EQUAL(ctx->trigger_src, expected[i]);
	}

	receive("TRIGGER:SOURCE WRONG");
	ASSERT_SCPI_ERROR(SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE);
	ASSERT_NO_RESPONSE();
}

static void test_sourceQ() {
	scpimm_context_t* const ctx = SCPIMM_context();
	const scpimm_trig_src_t values[] = {SCPIMM_TRIG_BUS, SCPIMM_TRIG_IMM, SCPIMM_TRIG_EXT};
	const char* expected[] = {"BUS\r\n", "IMM\r\n", "EXT\r\n"};
	size_t i;

	for (i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
		ctx->trigger_src = values[i];
		receive("TRIGGER:SOURCE?");
		ASSERT_NO_SCPI_ERRORS();
		ASSERT_RESPONSE(expected[i]);
	}
}

static void test_delay() {
	scpimm_context_t* const ctx = SCPIMM_context();
	float f;

	ctx->trigger_count_num = 0;
	ctx->infinite_trigger_count = TRUE;

	receive("TRIGGER:DELAY MIN");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_DOUBLE_EQUAL(ctx->trigger_delay, 0.0f, FLOAT_DELTA);

	receive("TRIGGER:DELAY MAX");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_DOUBLE_EQUAL(ctx->trigger_delay, 3600.0f, FLOAT_DELTA);

	for (f = 0.0f; f <= 3600.0f; f += 123.45f) {
		receivef("TRIGGER:DELAY %f", (double) f);
		ASSERT_NO_SCPI_ERRORS();
		ASSERT_NO_RESPONSE();
		CU_ASSERT_DOUBLE_EQUAL(ctx->trigger_delay, f, FLOAT_DELTA);
	}

	receive("TRIGGER:DELAY -1");
	ASSERT_SCPI_ERROR(SCPIMM_ERROR_DATA_OUT_OF_RANGE);
	ASSERT_NO_RESPONSE();

	receive("TRIGGER:DELAY 3601");
	ASSERT_SCPI_ERROR(SCPIMM_ERROR_DATA_OUT_OF_RANGE);
	ASSERT_NO_RESPONSE();

	receive("TRIGGER:COUNT DEF");
	ASSERT_SCPI_ERROR(SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE);
	ASSERT_NO_RESPONSE();
}

static void test_delayQ() {
	scpimm_context_t* const ctx = SCPIMM_context();
	double f;

	ctx->trigger_delay = 123.45;

	receive("TRIGGER:DELAY? MIN");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_DOUBLE_RESPONSE(0);

	receive("TRIGGER:DELAY? MAX");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_DOUBLE_RESPONSE(3600);

	ctx->infinite_trigger_count = FALSE;
	for (f = 0; f <= 3600; f += 123.45) {
		ctx->trigger_delay = f;
		receive("TRIGGER:DELAY?");
		ASSERT_NO_SCPI_ERRORS();
		ASSERT_DOUBLE_RESPONSE(f);
	}
}

static void test_delay_auto() {
	delay_auto_impl("TRIGGER:DELAY:AUTO 0", FALSE);
	delay_auto_impl("TRIGGER:DELAY:AUTO 1", TRUE);
	delay_auto_impl("TRIGGER:DELAY:AUTO +0", FALSE);
	delay_auto_impl("TRIGGER:DELAY:AUTO +1", TRUE);
	delay_auto_impl("TRIGGER:DELAY:AUTO OFF", FALSE);
	delay_auto_impl("TRIGGER:DELAY:AUTO ON", TRUE);
	delay_auto_impl("TRIGGER:DELAY:AUTO Off", FALSE);
	delay_auto_impl("TRIGGER:DELAY:AUTO On", TRUE);
}

static void test_delay_autoQ() {
	CONTEXT->trigger_auto_delay = FALSE;
	receive("TRIGGER:DELAY:AUTO?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_BOOL_RESPONSE(FALSE);

	CONTEXT->trigger_auto_delay = TRUE;
	receive("TRIGGER:DELAY:AUTO?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_BOOL_RESPONSE(TRUE);
}

static void test_count() {
	scpimm_context_t* const ctx = SCPIMM_context();
	int i;

	ctx->trigger_count_num = 0;
	ctx->infinite_trigger_count = TRUE;

	receive("TRIGGER:COUNT MIN");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(ctx->trigger_count_num, 1);
	CU_ASSERT_EQUAL(ctx->infinite_trigger_count, FALSE);

	receive("TRIGGER:COUNT INF");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(ctx->infinite_trigger_count, TRUE);

	receive("TRIGGER:COUNT MAX");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	CU_ASSERT_EQUAL(ctx->trigger_count_num, MAX_TRIGGER_COUNT);
	CU_ASSERT_EQUAL(ctx->infinite_trigger_count, FALSE);

	for (i = 1; i <= SCPIMM_BUF_CAPACITY; ++i) {
		receivef("TRIGGER:COUNT %d", i);
		ASSERT_NO_SCPI_ERRORS();
		ASSERT_NO_RESPONSE();
		CU_ASSERT_EQUAL((int) ctx->trigger_count_num, i);
		CU_ASSERT_EQUAL(ctx->infinite_trigger_count, FALSE);
	}

	receive("TRIGGER:COUNT 0");
	ASSERT_SCPI_ERROR(SCPIMM_ERROR_DATA_OUT_OF_RANGE);
	ASSERT_NO_RESPONSE();

	receivef("TRIGGER:COUNT %d", (int) (MAX_TRIGGER_COUNT + 1));
	ASSERT_SCPI_ERROR(SCPIMM_ERROR_DATA_OUT_OF_RANGE);
	ASSERT_NO_RESPONSE();

	receive("TRIGGER:COUNT DEF");
	ASSERT_SCPI_ERROR(SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE);
	ASSERT_NO_RESPONSE();
}

static void test_countQ() {
	scpimm_context_t* const ctx = SCPIMM_context();
	int i;

	receive("TRIGGER:COUNT? MIN");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_INT_RESPONSE(1);

	receive("TRIGGER:COUNT? MAX");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_INT_RESPONSE(MAX_TRIGGER_COUNT);

	ctx->infinite_trigger_count = FALSE;
	for (i = 1; i <= SCPIMM_BUF_CAPACITY; ++i) {
		ctx->trigger_count_num = i;
		receive("TRIGGER:COUNT?");
		ASSERT_NO_SCPI_ERRORS();
		ASSERT_INT_RESPONSE(i);
	}

	ctx->infinite_trigger_count = TRUE;
	receive("TRIGGER:COUNT?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_DOUBLE_RESPONSE(9.90000000E+37);
}

int test_trigger() {
    CU_pSuite pSuite = NULL;

    /* Add a suite to the registry */
    ADD_SUITE("TRIGGER");

    /* Add the tests to the suite */
    ADD_TEST(test_source);
    ADD_TEST(test_sourceQ);
    ADD_TEST(test_delay);
    ADD_TEST(test_delayQ);
    ADD_TEST(test_delay_auto);
    ADD_TEST(test_delay_autoQ);
    ADD_TEST(test_count);
    ADD_TEST(test_countQ);

    return 0;
}

#ifdef	NO_GLOBAL_TEST

int main() {

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    if (test_trigger()) {
    	return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    return RUN_ALL_TESTS();
}

#endif
