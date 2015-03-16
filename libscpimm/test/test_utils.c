#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "CUnit/Basic.h"
#include <scpimm/scpimm.h>
#include "test_utils.h"
#include "default_multimeter.h"

#define DOUBLE_DELTA 1.0e-6

static int scpi_error(scpi_t * context, int_fast16_t error);

void init_test_vars() {
	dm_init_in_buffer();
}

void init_scpimm() {
	SCPIMM_setup(&dm_interface);
	SCPI_context()->interface->error = scpi_error;
	clearscpi_errors();
}

void receive(const char* s) {
	dm_init_in_buffer();
//	printf("SEND: %s\n", s);
	SCPIMM_parseInBuffer(s, strlen(s));
	SCPIMM_parseInBuffer("\r\n", 2);
}

void receivef(const char* fmt, ...) {
	char buf[100];
	va_list params;

	va_start(params, fmt);
	vsprintf(buf, fmt, params);
	va_end(params);

	receive(buf);
}

void dump_in_data() {
	printf("IN DATA [%s] LENGTH [%u]\n", dm_output_buffer(), (unsigned) strlen(dm_output_buffer()));
}

void clearscpi_errors() {
	SCPI_ErrorClear(SCPI_context());
}

void assert_no_scpi_errors() {
	CU_ASSERT_EQUAL(SCPI_ErrorCount(SCPI_context()), 0);
}

void assert_scpi_error(int16_t error) {
	CU_ASSERT_EQUAL(SCPI_ErrorCount(SCPI_context()), 1);
	CU_ASSERT_EQUAL(SCPI_ErrorPop(SCPI_context()), error);
}

void assert_in_data(const char* s) {
	CU_ASSERT_STRING_EQUAL(dm_output_buffer(), s);
}

void assert_no_data() {
	assert_in_data("");
}

void assert_in_bool(bool_t v) {
	assert_in_data(v ? "1\r\n" : "0\r\n");
}

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

static int scpi_error(scpi_t * context, int_fast16_t error) {
	(void) context;
	(void) error;
//	printf("SCPI ERROR %d\n", (int) error);
	return 0;
}

void assert_number_equals(const scpi_number_t* v, const scpi_number_t* expected) {
    CU_ASSERT_EQUAL(v->type, expected->type);
	if (v->type == expected->type && v->type == SCPI_NUM_NUMBER) {
	    CU_ASSERT_EQUAL(v->unit, expected->unit);
		CU_ASSERT_DOUBLE_EQUAL(v->value, expected->value, expected->value * FLOAT_DELTA);
	}
}

const scpimm_interface_t* scpimm_interface() {
	return &dm_interface;
}
