#include <string.h>
#include <stdio.h>
#include "CUnit/Basic.h"
#include <scpimm/scpimm.h>
#include "test_utils.h"

static int set_mode(const uint8_t mode, float range, float resolution);
static void set_remote(bool_t remote, bool_t lock);
static size_t send(const uint8_t* data, const size_t len);

static char inbuffer[1024];
static char* inpuffer_pos = inbuffer;

static scpimm_interface_t scpimm_interface = {
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
}

void receive(const char* s) {
	init_in_buffer();
	SCPIMM_parseInBuffer(s, strlen(s));
	SCPIMM_parseInBuffer("\r\n", 2);
}

void dump_in_data() {
	printf("IN DATA [%s] LENGTH [%u]\n", inbuffer, strlen(inbuffer));
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

static int set_mode(const uint8_t mode, float range, float resolution) {
	return 0;	/* stub */
}

static void set_remote(bool_t remote, bool_t lock) {
	/* stub */
}

static size_t send(const uint8_t* data, const size_t len) {
	memcpy(inpuffer_pos, (const char*) data, len);
	inpuffer_pos += len;
	*inpuffer_pos = '\0';
	return len;
}

