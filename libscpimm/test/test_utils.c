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

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include "CUnit/Basic.h"
#include <scpimm/scpimm.h>
#include "test_utils.h"
#include "default_multimeter.h"

#define DOUBLE_DELTA 1.0e-6

int init_suite(void) {
	SCPIMM_setup(&dm_interface);

    return 0;
}

int clean_suite(void) {
    return 0;
}

void receive(const char* s) {
//	printf("SEND: %s\n", s);
	SCPIMM_parse_in_buffer(s, strlen(s));
	SCPIMM_parse_in_buffer("\r\n", 2);
}

void receivef(const char* fmt, ...) {
	char buf[100];
	va_list params;

	va_start(params, fmt);
	vsprintf(buf, fmt, params);
	va_end(params);

	receive(buf);
}

void clearscpi_errors() {
	SCPI_ErrorClear(SCPI_context());
}

/*
void assert_in_int(int v) {
	char buf[32];
	sprintf(buf, "%d\r\n", v);
	assert_in_data(buf);
}

void assert_in_double(double v) {
	char* endp;
	double d;
	d = strtod(dm_output_buffer(), &endp);
    CU_ASSERT_EQUAL(*endp, '\r');
	CU_ASSERT_DOUBLE_EQUAL(d, v, v * DOUBLE_DELTA);
}

void assert_number_equals(const scpi_number_t* v, const scpi_number_t* expected) {
    CU_ASSERT_EQUAL(v->type, expected->type);
	if (v->type == expected->type && v->type == SCPI_NUM_NUMBER) {
	    CU_ASSERT_EQUAL(v->unit, expected->unit);
		CU_ASSERT_DOUBLE_EQUAL(v->value, expected->value, expected->value * FLOAT_DELTA);
	}
}
*/

const scpimm_interface_t* scpimm_interface() {
	return &dm_interface;
}

void shorten_command_name(char* buf) {
	char* s;

	for (s = buf; *s; ) {
		if (isalpha(*s) && islower(*s)) {
			memmove(s, s + 1, strlen(s + 1) + 1);
		} else {
			s++;
		}
	}
}

static void call_test(const char* const func, const scpimm_mode_t mode, void (*test)(const char*, scpimm_mode_t mode, void* user_data), void* const user_data) {
	char buf[64];

	// full command name, e.g. VOLTage
	strcpy(buf, func);
	test(buf, mode, user_data);

	// short command name, e.g. VOLT
	shorten_command_name(buf);
	test(buf, mode, user_data);
}

void repeat_for_all_modes(void (*test)(const char*, scpimm_mode_t mode, void* user_data), void* const user_data) {
	call_test("FRESistance", SCPIMM_MODE_RESISTANCE_4W, test, user_data);
	call_test("VOLTage", SCPIMM_MODE_DCV, test, user_data);
	call_test("VOLTage:DC", SCPIMM_MODE_DCV, test, user_data);
	call_test("VOLTage:RATio", SCPIMM_MODE_DCV_RATIO, test, user_data);
	call_test("VOLTage:DC:RATio", SCPIMM_MODE_DCV_RATIO, test, user_data);
	call_test("VOLTage:AC", SCPIMM_MODE_ACV, test, user_data);
	call_test("VOLTage:AC:RATio", SCPIMM_MODE_ACV_RATIO, test, user_data);
	call_test("CURRent", SCPIMM_MODE_DCC, test, user_data);
	call_test("CURRent:DC", SCPIMM_MODE_DCC, test, user_data);
	call_test("CURRent:AC", SCPIMM_MODE_ACC, test, user_data);
	call_test("RESistance", SCPIMM_MODE_RESISTANCE_2W, test, user_data);
}

void repeat_for_all_dc_modes(void (*test)(const char*, scpimm_mode_t mode, void* user_data), void* const user_data) {
	call_test("FRESistance", SCPIMM_MODE_RESISTANCE_4W, test, user_data);
	call_test("VOLTage", SCPIMM_MODE_DCV, test, user_data);
	call_test("VOLTage:DC", SCPIMM_MODE_DCV, test, user_data);
	call_test("VOLTage:RATio", SCPIMM_MODE_DCV_RATIO, test, user_data);
	call_test("VOLTage:DC:RATio", SCPIMM_MODE_DCV_RATIO, test, user_data);
	call_test("CURRent", SCPIMM_MODE_DCC, test, user_data);
	call_test("CURRent:DC", SCPIMM_MODE_DCC, test, user_data);
	call_test("RESistance", SCPIMM_MODE_RESISTANCE_2W, test, user_data);
}

#ifndef	NO_GLOBAL_TEST

int test_configure();
int test_display();
int test_dmm();
int test_generic();
int test_ieee488();
int test_input();
int test_route();
int test_sample();
int test_sense();
int test_trigger();

int main() {
    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    if (
    		test_configure()
    		|| test_display()
    		|| test_dmm()
    		|| test_generic()
    		|| test_ieee488()
    		|| test_input()
    		|| test_route()
    		|| test_sample()
    		|| test_sense()
    		|| test_trigger()
    	) {
    	return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    return RUN_ALL_TESTS();
}

#endif
