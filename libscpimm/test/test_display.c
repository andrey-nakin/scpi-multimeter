#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "test_utils.h"

#define CONTEXT (SCPIMM_context())

static void delay_auto_impl(const char* cmd, bool_t expected) {
	receive(cmd);
	assert_no_scpi_errors();
	asset_no_data();
    CU_ASSERT_EQUAL(CONTEXT->display, expected);
}

int init_suite(void) {
    return 0;
}

int clean_suite(void) {
    return 0;
}

void test_display() {
	init_scpimm();

	delay_auto_impl("DISPLAY 0", FALSE);
	delay_auto_impl("DISPLAY 1", TRUE);
	delay_auto_impl("DISPLAY +0", FALSE);
	delay_auto_impl("DISPLAY +1", TRUE);
	delay_auto_impl("DISPLAY OFF", FALSE);
	delay_auto_impl("DISPLAY ON", TRUE);
	delay_auto_impl("DISPLAY Off", FALSE);
	delay_auto_impl("DISPLAY On", TRUE);
}

void test_displayQ() {
	init_scpimm();

	CONTEXT->display = FALSE;
	receive("DISPLAY?");
	assert_no_scpi_errors();
	asset_in_bool(FALSE);

	CONTEXT->display = TRUE;
	receive("DISPLAY?");
	assert_no_scpi_errors();
	asset_in_bool(TRUE);
}

void test_display_text() {
	init_scpimm();

	receive("DISPLAY:TEXT ''");
	assert_no_scpi_errors();
	asset_no_data();
    CU_ASSERT_STRING_EQUAL(CONTEXT->display_text, "");

	receive("DISPLAY:TEXT '1234567890AB'");
	assert_no_scpi_errors();
	asset_no_data();
    CU_ASSERT_STRING_EQUAL(CONTEXT->display_text, "1234567890AB");

	receive("DISPLAY:TEXT '1234567890ABC'");
	assert_scpi_error(SCPI_ERROR_TOO_MUCH_DATA);
	asset_no_data();
}

void test_display_textQ() {
	init_scpimm();

	receive("DISPLAY:TEXT?");
	assert_no_scpi_errors();
	asset_in_data("\"\"\r\n");

	strcpy(CONTEXT->display_text, "1234567890AB");
	receive("DISPLAY:TEXT?");
	assert_no_scpi_errors();
	asset_in_data("\"1234567890AB\"\r\n");
}

void test_display_text_clear() {
	init_scpimm();

	strcpy(CONTEXT->display_text, "1234567890AB");
	receive("DISPLAY:TEXT:CLEAR");
	assert_no_scpi_errors();
	asset_no_data();
    CU_ASSERT_STRING_EQUAL(CONTEXT->display_text, "");
}

int main() {
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    pSuite = CU_add_suite("DISPLAY", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "test display", test_display))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test display?", test_displayQ))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test display:text", test_display_text))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test display:text?", test_display_textQ))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test display:text:clear", test_display_text_clear))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

