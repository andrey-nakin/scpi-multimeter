#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "test_utils.h"
#include "default_multimeter.h"

static void read_numbers(const unsigned expected_num_of_values, double* values) {
	const char * const result = dm_read_entire_output_buffer(), *s;
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

static void read_data_points(const unsigned expected_num_of_values) {
	const char* const result = dm_read_entire_output_buffer();
	int pos = 0;
	double value;

	CU_ASSERT_EQUAL(sscanf(result, "%lg%n", &value, &pos), 1);
	CU_ASSERT_STRING_EQUAL(result + pos, "\r\n");
	CU_ASSERT_EQUAL((unsigned) value, expected_num_of_values);
}

static void read_equal_numbers(const unsigned expected_num_of_values, double* const values, const double expected_value) {
	size_t i;

	read_numbers(expected_num_of_values, values);
	for (i = 0; i < expected_num_of_values; i++) {
		ASSERT_DOUBLE_EQUAL(values[i], expected_value);
	}
}

void check_after_read_state() {
	ASSERT_NO_SCPI_ERRORS();
	CU_ASSERT_EQUAL(SCPIMM_context()->state, SCPIMM_STATE_IDLE);
	ASSERT_INTERRUPTS_ARE_ENABLED();
}

void test_readQ_generic_impl(const dm_measurement_type_t mt, const char* const trigger_src) {
	const char* const result = dm_read_entire_output_buffer();
	double actual_range, actual_resolution;
	double values[16];
	const int sample_count = 5, trigger_count = 3;

	dm_multimeter_config.measurement_type = mt;

	// read current value range
	receivef("CONFIGURE?");
	ASSERT_NO_SCPI_ERRORS();
	CU_ASSERT_EQUAL(sscanf(strchr(result, ' ') + 1, "%le,%le", &actual_range, &actual_resolution), 2);

	// read single value
	receivef("TRIGGER:SOURCE %s", trigger_src);
	dm_reset_counters();
	receivef("READ?");
	check_after_read_state();
	read_equal_numbers(1, values, actual_range * 0.5);

	// decrease measurement duration for quicker testing
	dm_multimeter_config.measurement_duration = 10;

	// attempt to read value with invalid trigger source
	receivef("TRIGGER:SOURCE BUS");
	ASSERT_NO_SCPI_ERRORS();
	dm_reset_counters();
	receivef("READ?");
	ASSERT_SCPI_ERROR(SCPI_ERROR_TRIGGER_DEADLOCK);

	// read SAMPLE_COUNT values
	receivef("TRIGGER:SOURCE %s", trigger_src);
	receivef("SAMPLE:COUNT %d", sample_count);
	ASSERT_NO_SCPI_ERRORS();
	dm_reset_counters();
	receivef("READ?");
	check_after_read_state();
	read_equal_numbers(sample_count, values, actual_range * 0.5);

	// read TRIGGER_COUNT values
	receivef("SAMPLE:COUNT %d", 1);
	ASSERT_NO_SCPI_ERRORS();
	receivef("TRIGGER:COUNT %d", trigger_count);
	ASSERT_NO_SCPI_ERRORS();
	dm_reset_counters();
	receivef("READ?");
	check_after_read_state();
	read_equal_numbers(trigger_count, values, actual_range * 0.5);

	// read SAMPLE_COUNT * TRIGGER)COUNT values
	receivef("SAMPLE:COUNT %d", sample_count);
	ASSERT_NO_SCPI_ERRORS();
	dm_reset_counters();
	receivef("READ?");
	check_after_read_state();
	read_equal_numbers(sample_count * trigger_count, values, actual_range * 0.5);

	// read value with failed hardware
	SCPIMM_context()->measurement_timeout = 100;	//	reduce timeout for quicker testing
	dm_multimeter_state.measurement_failure_counter = sample_count * trigger_count / 2;
	CU_ASSERT_TRUE(dm_multimeter_state.measurement_failure_counter > 0);
	receivef("READ?");
	ASSERT_SCPI_ERROR(SCPI_ERROR_IO_PROCESSOR_DOES_NOT_RESPOND);
	check_after_read_state();

	// read values again after failure
	dm_reset_counters();
	receivef("READ?");
	check_after_read_state();
	read_equal_numbers(sample_count * trigger_count, values, actual_range * 0.5);
}

void test_readQ() {
	test_readQ_generic_impl(DM_MEASUREMENT_TYPE_ASYNC, "IMM");
	test_readQ_generic_impl(DM_MEASUREMENT_TYPE_SYNC, "IMM");
	//!!!test_readQ_generic_impl(DM_MEASUREMENT_TYPE_ASYNC, "EXT");
	//!!!test_readQ_generic_impl(DM_MEASUREMENT_TYPE_SYNC, "EXT");
}

void check_after_init_state() {
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
}

void check_after_data_points_state() {
	check_after_read_state();
}

void check_after_fetch_state() {
	check_after_read_state();
}

void test_initiate_generic_impl(const dm_measurement_type_t mt, const char* const trigger_src) {
	const char* const result = dm_read_entire_output_buffer();
	double actual_range, actual_resolution;
	double values[16];
	const int sample_count = 2, trigger_count = 3;

	dm_multimeter_config.measurement_type = mt;

	// read current value range
	receivef("CONFIGURE?");
	ASSERT_NO_SCPI_ERRORS();
	CU_ASSERT_EQUAL(sscanf(strchr(result, ' ') + 1, "%le,%le", &actual_range, &actual_resolution), 2);

	// read single value
	receivef("TRIGGER:SOURCE %s", trigger_src);
	dm_reset_counters();
	receivef("INITIATE");
	check_after_init_state();
	receivef("FETCH?");
	check_after_fetch_state();
	read_equal_numbers(1, values, actual_range * 0.5);
	receivef("DATA:POINTS?");
	check_after_data_points_state();
	read_data_points(1);

	// decrease measurement duration for quicker testing
	dm_multimeter_config.measurement_duration = 10;

	// read SAMPLE_COUNT values
	receivef("SAMPLE:COUNT %d", sample_count);
	ASSERT_NO_SCPI_ERRORS();
	dm_reset_counters();
	receivef("INITIATE");
	check_after_init_state();
	receivef("FETCH?");
	check_after_fetch_state();
	read_equal_numbers(sample_count, values, actual_range * 0.5);
	receivef("DATA:POINTS?");
	check_after_data_points_state();
	read_data_points(sample_count);

	// read TRIGGER_COUNT values
	receivef("SAMPLE:COUNT %d", 1);
	ASSERT_NO_SCPI_ERRORS();
	receivef("TRIGGER:COUNT %d", trigger_count);
	ASSERT_NO_SCPI_ERRORS();
	dm_reset_counters();
	receivef("INITIATE");
	check_after_init_state();
	receivef("FETCH?");
	check_after_fetch_state();
	read_equal_numbers(trigger_count, values, actual_range * 0.5);
	receivef("DATA:POINTS?");
	check_after_data_points_state();
	read_data_points(trigger_count);

	// read SAMPLE_COUNT * TRIGGER_COUNT values
	receivef("SAMPLE:COUNT %d", sample_count);
	ASSERT_NO_SCPI_ERRORS();
	dm_reset_counters();
	receivef("INITIATE");
	check_after_init_state();
	receivef("FETCH?");
	check_after_fetch_state();
	read_equal_numbers(sample_count * trigger_count, values, actual_range * 0.5);
	receivef("DATA:POINTS?");
	check_after_data_points_state();
	read_data_points(sample_count * trigger_count);

	// read value with failed hardware
	SCPIMM_context()->measurement_timeout = 100;	//	reduce timeout for quicker testing
	dm_multimeter_state.measurement_failure_counter = sample_count * trigger_count / 2;
	CU_ASSERT_TRUE(dm_multimeter_state.measurement_failure_counter > 0);
	receivef("INITIATE");
	check_after_init_state();
	receivef("FETCH?");
	ASSERT_SCPI_ERROR(SCPI_ERROR_IO_PROCESSOR_DOES_NOT_RESPOND);

	// read values again after failure
	dm_reset_counters();
	receivef("INITIATE");
	check_after_init_state();
	receivef("FETCH?");
	check_after_fetch_state();
	read_equal_numbers(sample_count * trigger_count, values, actual_range * 0.5);
	receivef("DATA:POINTS?");
	check_after_data_points_state();
	read_data_points(sample_count * trigger_count);

	// check "out-of-buffer-length" error
	receivef("TRIGGER:COUNT %u", 1);
	ASSERT_NO_SCPI_ERRORS();
	receivef("SAMPLE:COUNT %u", (unsigned) (SCPIMM_BUF_CAPACITY + 1));
	ASSERT_NO_SCPI_ERRORS();
	receivef("INITIATE");
	ASSERT_SCPI_ERROR(SCPI_ERROR_INSUFFICIENT_MEMORY);
}

void test_initiate_bus_trigger(const dm_measurement_type_t mt) {
	const char* result;
	double actual_range, actual_resolution;
	double values[16];

	dm_multimeter_config.measurement_type = mt;
	// decrease measurement duration for quicker testing
	dm_multimeter_config.measurement_duration = 10;

	// read current value range
	receivef("CONFIGURE?");
	ASSERT_NO_SCPI_ERRORS();
	result = dm_read_entire_output_buffer();
	CU_ASSERT_EQUAL(sscanf(strchr(result, ' ') + 1, "%le,%le", &actual_range, &actual_resolution), 2);

	// read single value from BUS trigger
	receivef("TRIGGER:SOURCE BUS");
	dm_reset_counters();
	receivef("INITIATE");
	check_after_init_state();
	receivef("*TRG");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	receivef("FETCH?");
	check_after_fetch_state();
	read_equal_numbers(1, values, actual_range * 0.5);
	receivef("DATA:POINTS?");
	check_after_data_points_state();
	read_data_points(1);

	// unexpected *TRG command
	receivef("*TRG");
	ASSERT_SCPI_ERROR(SCPI_ERROR_TRIGGER_IGNORED);
	ASSERT_NO_RESPONSE();

	// read single value from IMM trigger with unexpected *TRG command
	receivef("TRIGGER:SOURCE IMM");
	dm_reset_counters();
	receivef("INITIATE");
	check_after_init_state();
	receivef("*TRG");
	ASSERT_SCPI_ERROR(SCPI_ERROR_TRIGGER_IGNORED);
	ASSERT_NO_RESPONSE();
	receivef("FETCH?");
	check_after_fetch_state();
	read_equal_numbers(1, values, actual_range * 0.5);
	receivef("DATA:POINTS?");
	check_after_data_points_state();
	read_data_points(1);
}

void test_initiate() {
	test_initiate_generic_impl(DM_MEASUREMENT_TYPE_ASYNC, "IMM");
	test_initiate_generic_impl(DM_MEASUREMENT_TYPE_SYNC, "IMM");
	test_initiate_generic_impl(DM_MEASUREMENT_TYPE_ASYNC, "EXT");
	test_initiate_generic_impl(DM_MEASUREMENT_TYPE_SYNC, "EXT");

	test_initiate_bus_trigger(DM_MEASUREMENT_TYPE_ASYNC);
	test_initiate_bus_trigger(DM_MEASUREMENT_TYPE_SYNC);
}

void test_unexpected_fetch() {
	receivef("FETCH?");
	ASSERT_SCPI_ERROR(SCPI_ERROR_DATA_STALE);
	ASSERT_NO_RESPONSE();
}

void test_fetch() {
	test_unexpected_fetch();
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
    if ((NULL == CU_add_test(pSuite, "READ?", test_readQ))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    /*!!!if ((NULL == CU_add_test(pSuite, "INITiate", test_initiate))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "FETCh?", test_fetch))) {
        CU_cleanup_registry();
        return CU_get_error();
    }*/

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

