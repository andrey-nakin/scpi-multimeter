#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "CUnit/Basic.h"
#include <scpimm/scpimm.h>
#include "test_utils.h"

#define DOUBLE_DELTA 1.0e-6

static scpimm_mode_t supported_modes(void);
static int16_t set_mode(scpimm_mode_t mode, const scpi_number_t* range, const scpi_number_t* resolution);
static void set_remote(bool_t remote, bool_t lock);
static size_t send(const uint8_t* data, const size_t len);
static int scpi_error(scpi_t * context, int_fast16_t error);

static char inbuffer[1024];
static char* inpuffer_pos = inbuffer;

static scpimm_interface_t scpimm_interface = {
	.supported_modes = supported_modes,
	.set_mode = set_mode,
	.send = send,
	.remote = set_remote
};

void init_in_buffer() {
	inpuffer_pos = inbuffer;
	*inpuffer_pos = '\0';
}

void init_test_vars() {
	init_in_buffer();
}

void init_scpimm() {
	SCPIMM_setup(&scpimm_interface);
	SCPI_context()->interface->error = scpi_error;
	clearscpi_errors();
}

void receive(const char* s) {
	init_in_buffer();
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
	printf("IN DATA [%s] LENGTH [%u]\n", inbuffer, (unsigned) strlen(inbuffer));
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

void asset_in_data(const char* s) {
    CU_ASSERT_EQUAL(strcmp(s, inbuffer), 0);
}

void asset_no_data() {
	asset_in_data("");
}

void asset_in_bool(bool_t v) {
	asset_in_data(v ? "1\r\n" : "0\r\n");
}

void assert_in_int(int v) {
	char buf[32];
	sprintf(buf, "%d\r\n", v);
	asset_in_data(buf);
}

void assert_in_double(double v) {
	char* endp;
	double d;
	d = strtod(inbuffer, &endp);
    CU_ASSERT_EQUAL(*endp, '\r');
	CU_ASSERT_DOUBLE_EQUAL(d, v, v * DOUBLE_DELTA);
}

static scpimm_mode_t supported_modes(void) {
	/* all modes are supported */
	return (scpimm_mode_t) -1;
}

static int16_t set_mode(scpimm_mode_t mode, const scpi_number_t* range, const scpi_number_t* resolution) {
	(void) mode;
	(void) range;
	(void) resolution;
	return SCPI_ERROR_OK;	/* stub */
}

static void set_remote(bool_t remote, bool_t lock) {
	(void) remote;
	(void) lock;
	/* stub */
}

static size_t send(const uint8_t* data, const size_t len) {
	memcpy(inpuffer_pos, (const char*) data, len);
	inpuffer_pos += len;
	*inpuffer_pos = '\0';
	return len;
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

