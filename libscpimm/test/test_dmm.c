#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "test_utils.h"
#include "default_multimeter.h"

static void reset() {
	init_scpimm();
	clearscpi_errors();
	init_test_vars();
}

int init_suite(void) {
    return 0;
}

int clean_suite(void) {
    return 0;
}

static void read_numbers(const unsigned expected_num_of_values, double* values) {
	char * const result = dm_output_buffer(), *s;
	int pos = 0;
	unsigned actual_num_of_values;
	double value;

	CU_ASSERT_EQUAL(dm_counters.start_measure, expected_num_of_values);
	for (actual_num_of_values = 1, s = result; *s; s++) {
		if (',' == *s) {
			actual_num_of_values++;
		}
	}
	CU_ASSERT_EQUAL(actual_num_of_values, expected_num_of_values);

	s = result;
	while (actual_num_of_values--) {
		CU_ASSERT_EQUAL(sscanf(s, "%le%n", &value, &pos), 1);
		if (values) {
			*values++ = value;
		}
		s += pos;
		if (actual_num_of_values) {
			CU_ASSERT_EQUAL(*s++, ',');
		}
	}
	CU_ASSERT_STRING_EQUAL(s, "\r\n");
}

static void read_equal_numbers(const unsigned expected_num_of_values, double* const values, const double expected_value) {
	size_t i;

	read_numbers(expected_num_of_values, values);
	for (i = 0; i < expected_num_of_values; i++) {
		ASSERT_DOUBLE_EQUAL(values[i], expected_value);
	}
}

void test_readQ_generic_impl(const dm_measurement_type_t mt) {
	char *result = dm_output_buffer();
	double actual_range, actual_resolution;
	double values[16];
	const int sample_count = 5, trigger_count = 3;

	reset();
	dm_multimeter_config.measurement_type = mt;

	receivef("CONFIGURE?");
	assert_no_scpi_errors();
	CU_ASSERT_EQUAL(sscanf(strchr(result, ' ') + 1, "%le,%le", &actual_range, &actual_resolution), 2);

	dm_reset_counters();
	receivef("READ?");
	assert_no_scpi_errors();
	ASSERT_INTERRUPTS_ARE_ENABLED();
	read_equal_numbers(1, values, actual_range * 0.5);

	dm_multimeter_config.measurement_duration = 10;

	receivef("SAMPLE:COUNT %d", sample_count);
	assert_no_scpi_errors();
	dm_reset_counters();
	receivef("READ?");
	assert_no_scpi_errors();
	ASSERT_INTERRUPTS_ARE_ENABLED();
	read_equal_numbers(sample_count, values, actual_range * 0.5);

	receivef("SAMPLE:COUNT %d", 1);
	assert_no_scpi_errors();
	receivef("TRIGGER:COUNT %d", trigger_count);
	assert_no_scpi_errors();
	dm_reset_counters();
	receivef("READ?");
	assert_no_scpi_errors();
	ASSERT_INTERRUPTS_ARE_ENABLED();
	read_equal_numbers(trigger_count, values, actual_range * 0.5);

	receivef("SAMPLE:COUNT %d", sample_count);
	assert_no_scpi_errors();
	dm_reset_counters();
	receivef("READ?");
	assert_no_scpi_errors();
	ASSERT_INTERRUPTS_ARE_ENABLED();
	read_equal_numbers(sample_count * trigger_count, values, actual_range * 0.5);

	dm_multimeter_state.measurement_failure_counter = sample_count * trigger_count / 2;
	CU_ASSERT_TRUE(dm_multimeter_state.measurement_failure_counter > 0);
	receivef("READ?");
	assert_scpi_error(SCPI_ERROR_IO_PROCESSOR_DOES_NOT_RESPOND);
	ASSERT_INTERRUPTS_ARE_ENABLED();

	// TODO check bus trigger error
	// TODO check READ? after a failure
}

void test_readQ_generic() {
	test_readQ_generic_impl(DM_MEASUREMENT_TYPE_ASYNC);
	test_readQ_generic_impl(DM_MEASUREMENT_TYPE_SYNC);
}

void test_readQ() {
//	test_readQ_generic();
}

int main() {
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    pSuite = CU_add_suite("DMM", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "read? generic", test_readQ_generic))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

