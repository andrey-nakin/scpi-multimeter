#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "CUnit/Basic.h"
#include <scpimm/scpimm.h>
#include "test_utils.h"

static scpimm_mode_t supported_modes(void);
static bool_t set_mode(const scpimm_mode_t mode);
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

void asset_in_data(const char* s) {
    CU_ASSERT_EQUAL(strcmp(s, inbuffer), 0);
}

void asset_no_data() {
	asset_in_data("");
}

void asset_in_bool(bool_t v) {
	asset_in_data(v ? "1\r\n" : "0\r\n");
}

static scpimm_mode_t supported_modes(void) {
	/* all modes are supported */
	return (scpimm_mode_t) -1;
}

static bool_t set_mode(const scpimm_mode_t mode) {
	(void) mode;
	return TRUE;	/* stub */
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

