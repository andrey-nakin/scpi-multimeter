#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include "CUnit/Basic.h"
#include <scpimm/scpimm.h>
#include "test_utils.h"
#include "default_multimeter.h"

#define DOUBLE_DELTA 1.0e-6

static int scpi_error(scpi_t * context, int_fast16_t error);

int init_suite(void) {
	SCPIMM_setup(&dm_interface);
	SCPI_context()->interface->error = scpi_error;

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

static int scpi_error(scpi_t * context, int_fast16_t error) {
	(void) context;
	(void) error;
//	printf("SCPI ERROR %d\n", (int) error);
	return 0;
}

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
	call_test("VOLTage:DC:RATio", SCPIMM_MODE_DCV_RATIO, test, user_data);
	call_test("VOLTage:AC", SCPIMM_MODE_ACV, test, user_data);
	call_test("CURRent", SCPIMM_MODE_DCC, test, user_data);
	call_test("CURRent:DC", SCPIMM_MODE_DCC, test, user_data);
	call_test("CURRent:AC", SCPIMM_MODE_ACC, test, user_data);
	call_test("RESistance", SCPIMM_MODE_RESISTANCE_2W, test, user_data);
}

void repeat_for_all_dc_modes(void (*test)(const char*, scpimm_mode_t mode, void* user_data), void* const user_data) {
	call_test("FRESistance", SCPIMM_MODE_RESISTANCE_4W, test, user_data);
	call_test("VOLTage", SCPIMM_MODE_DCV, test, user_data);
	call_test("VOLTage:DC", SCPIMM_MODE_DCV, test, user_data);
	call_test("VOLTage:DC:RATio", SCPIMM_MODE_DCV_RATIO, test, user_data);
	call_test("CURRent", SCPIMM_MODE_DCC, test, user_data);
	call_test("CURRent:DC", SCPIMM_MODE_DCC, test, user_data);
	call_test("RESistance", SCPIMM_MODE_RESISTANCE_2W, test, user_data);
}
