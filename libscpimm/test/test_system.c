#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "test_utils.h"

#define CONTEXT (SCPIMM_context())

static unsigned beeper_counter = 0;
static unsigned remove_counter = 0;
static bool_t last_remote = 0xff;
static bool_t last_lock = 0xff;

static void beeper() {
	++beeper_counter;
}

static int16_t remote(bool_t remote, bool_t lock) {
	++remove_counter;
	last_remote = remote;
	last_lock = lock;
	return SCPI_ERROR_OK;
}

static void beeper_state_impl(const char* cmd, bool_t expected) {
	receive(cmd);
	assert_no_scpi_errors();
	assert_no_data();
    CU_ASSERT_EQUAL(CONTEXT->beeper_state, expected);
}

static void remote_impl(const char* cmd, bool_t expected_remote, bool_t expected_lock) {
	remove_counter = 0;
	last_remote = 0xff;
	last_lock = 0xff;

	receive(cmd);
	assert_no_scpi_errors();
	assert_no_data();

    CU_ASSERT_EQUAL(remove_counter, 1);
    CU_ASSERT_EQUAL(last_remote, expected_remote);
    CU_ASSERT_EQUAL(last_lock, expected_lock);
}

static void error_impl(const char* cmd) {
	receive(cmd);
	assert_in_data("0, \"No error\"\r\n");

	receive("wrong command");
	receive(cmd);
	assert_in_data("-113, \"Undefined header\"\r\n");

	receive(cmd);
	assert_in_data("0, \"No error\"\r\n");
}

int init_suite(void) {
	init_scpimm();
	CONTEXT->interface->beep = beeper;
	CONTEXT->interface->remote = remote;
	clearscpi_errors();
	init_test_vars();
    return 0;
}

int clean_suite(void) {
    return 0;
}

void test_beeper() {
	/* issue single beep */
	beeper_counter = 0;
	receive("SYSTEM:BEEPER");
	assert_no_scpi_errors();
	assert_no_data();
    CU_ASSERT_EQUAL(beeper_counter, 1);
}

void test_beeper_state() {
	beeper_state_impl("SYSTEM:BEEPER:STATE 0", FALSE);
	beeper_state_impl("SYSTEM:BEEPER:STATE 1", TRUE);
	beeper_state_impl("SYSTEM:BEEPER:STATE +0", FALSE);
	beeper_state_impl("SYSTEM:BEEPER:STATE +1", TRUE);
	beeper_state_impl("SYSTEM:BEEPER:STATE OFF", FALSE);
	beeper_state_impl("SYSTEM:BEEPER:STATE ON", TRUE);
	beeper_state_impl("SYSTEM:BEEPER:STATE Off", FALSE);
	beeper_state_impl("SYSTEM:BEEPER:STATE On", TRUE);
}

void test_beeper_stateQ() {
	CONTEXT->beeper_state = FALSE;
	receive("SYSTEM:BEEPER:STATE?");
	assert_no_scpi_errors();
	assert_in_bool(FALSE);

	CONTEXT->beeper_state = TRUE;
	receive("SYSTEM:BEEPER:STATE?");
	assert_no_scpi_errors();
	assert_in_bool(TRUE);
}

void test_errorQ() {
	error_impl("SYSTEM:ERROR?");
}

void test_error_nextQ() {
	error_impl("SYSTEM:ERROR:NEXT?");
}

void test_error_countQ() {
	receive("SYSTEM:ERROR:COUNT?");
	assert_in_data("0\r\n");

	receive("wrong command");
	receive("SYSTEM:ERROR:COUNT?");
	assert_in_data("1\r\n");

	receive("wrong command 2");
	receive("SYSTEM:ERROR:COUNT?");
	assert_in_data("2\r\n");

	clearscpi_errors();
}

void test_local() {
	remote_impl("SYSTEM:LOCAL", FALSE, FALSE);
}

void test_remote() {
	remote_impl("SYSTEM:REMOTE", TRUE, FALSE);
}

void test_rwlock() {
	remote_impl("SYSTEM:RWLOCK", TRUE, TRUE);
}

void test_version() {
	receive("SYSTEM:VERSION?");
	assert_no_scpi_errors();
	assert_in_data("1992.0\r\n");
}

int main() {
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    pSuite = CU_add_suite("SYSTEM", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "test system:beeper", test_beeper))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test system:beeper:state", test_beeper_state))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test system:beeper:state?", test_beeper_stateQ))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test system:error?", test_errorQ))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test system:error:next?", test_error_nextQ))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test system:error:count?", test_error_countQ))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test system:local", test_local))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test system:remote", test_remote))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test system:rwlock", test_rwlock))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test system:version", test_version))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

