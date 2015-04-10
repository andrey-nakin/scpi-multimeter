#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "test_utils.h"

#define CONTEXT (SCPIMM_context())

static void beeper_state_impl(const char* cmd, scpi_bool_t expected) {
	receive(cmd);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
    CU_ASSERT_EQUAL(CONTEXT->beeper_state, expected);
}

static void remote_impl(const char* cmd, scpi_bool_t expected_remote, scpi_bool_t expected_lock) {
	dm_reset_counters();
	dm_reset_args();

	receive(cmd);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();

    CU_ASSERT_EQUAL(dm_counters.set_global_bool_param, 2);
    CU_ASSERT_EQUAL(dm_prev_args.set_global_bool_param.param, SCPIMM_PARAM_REMOTE);
    CU_ASSERT_EQUAL(dm_prev_args.set_global_bool_param.value, expected_remote);
    CU_ASSERT_EQUAL(dm_args.set_global_bool_param.param, SCPIMM_PARAM_LOCK);
    CU_ASSERT_EQUAL(dm_args.set_global_bool_param.value, expected_lock);
}

static void error_impl(const char* cmd) {
	receive(cmd);
	ASSERT_RESPONSE("0, \"No error\"\r\n");

	receive("wrong command");
	receive(cmd);
	ASSERT_RESPONSE("-113, \"Undefined header\"\r\n");

	receive(cmd);
	ASSERT_RESPONSE("0, \"No error\"\r\n");
}

static void test_beeper() {
	/* issue single beep */
	dm_reset_counters();

	receive("SYSTEM:BEEPER");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
    CU_ASSERT_EQUAL(dm_counters.beep, 1);
}

static void test_beeper_state() {
	beeper_state_impl("SYSTEM:BEEPER:STATE 0", FALSE);
	beeper_state_impl("SYSTEM:BEEPER:STATE 1", TRUE);
	beeper_state_impl("SYSTEM:BEEPER:STATE +0", FALSE);
	beeper_state_impl("SYSTEM:BEEPER:STATE +1", TRUE);
	beeper_state_impl("SYSTEM:BEEPER:STATE OFF", FALSE);
	beeper_state_impl("SYSTEM:BEEPER:STATE ON", TRUE);
	beeper_state_impl("SYSTEM:BEEPER:STATE Off", FALSE);
	beeper_state_impl("SYSTEM:BEEPER:STATE On", TRUE);
}

static void test_beeper_stateQ() {
	CONTEXT->beeper_state = FALSE;
	receive("SYSTEM:BEEPER:STATE?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_BOOL_RESPONSE(FALSE);

	CONTEXT->beeper_state = TRUE;
	receive("SYSTEM:BEEPER:STATE?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_BOOL_RESPONSE(TRUE);
}

static void test_errorQ() {
	error_impl("SYSTEM:ERROR?");
}

static void test_error_nextQ() {
	error_impl("SYSTEM:ERROR:NEXT?");
}

static void test_error_countQ() {
	receive("SYSTEM:ERROR:COUNT?");
	ASSERT_RESPONSE("0\r\n");

	receive("wrong command");
	receive("SYSTEM:ERROR:COUNT?");
	ASSERT_RESPONSE("1\r\n");

	receive("wrong command 2");
	receive("SYSTEM:ERROR:COUNT?");
	ASSERT_RESPONSE("2\r\n");

	clearscpi_errors();
}

static void test_local() {
	remote_impl("SYSTEM:LOCAL", FALSE, FALSE);
}

static void test_remote() {
	remote_impl("SYSTEM:REMOTE", TRUE, FALSE);
}

static void test_rwlock() {
	remote_impl("SYSTEM:RWLOCK", TRUE, TRUE);
}

static void test_version() {
	receive("SYSTEM:VERSION?");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_RESPONSE("1992.0\r\n");
}

int test_system() {
    CU_pSuite pSuite = NULL;

    /* Add a suite to the registry */
    ADD_SUITE("SYSTEM");

    /* Add the tests to the suite */
    ADD_TEST(test_beeper);
    ADD_TEST(test_beeper_state);
    ADD_TEST(test_beeper_stateQ);
    ADD_TEST(test_errorQ);
    ADD_TEST(test_error_nextQ);
    ADD_TEST(test_error_countQ);
    ADD_TEST(test_local);
    ADD_TEST(test_remote);
    ADD_TEST(test_rwlock);
    ADD_TEST(test_version);

    return 0;
}

#ifdef	NO_GLOBAL_TEST

int main() {
    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    if (test_system()) {
    	return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    return RUN_ALL_TESTS();
}

#endif
