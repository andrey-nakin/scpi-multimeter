#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include <scpi/scpi.h>
#include "test_utils.h"

#define	MIN_RANGE_INDEX	0
#define	MIN_RESOLUTION_INDEX	0

static const char* voltage_prefixes[] = {"V", "kV", "KV", "mV", "MV", "uV", "UV", NULL};
static const double voltage_mults[] = {1, 1e3, 1e3, 1e-3, 1e-3, 1e-6, 1e-6, 0.0};
static const char* current_prefixes[] = {"A", "kA", "KA", "mA", "MA", "uA", "UA", NULL};
static const double current_mults[] = {1, 1e3, 1e3, 1e-3, 1e-3, 1e-6, 1e-6, 0.0};
static const char* resistance_prefixes[] = {"Ohm", "kOhm", "KOhm", "mOhm", "MOhm", NULL};
static const double resistance_mults[] = {1, 1e3, 1e3, 1e6, 1e6, 0.0};

/* general checking after CONFIGURE command */
static void check_general(const scpimm_mode_t mode) {
	scpimm_mode_t cur_mode;
	scpimm_mode_params_t cur_params;
	const scpi_bool_t no_params = FALSE;	//	SCPIMM_MODE_CONTINUITY == mode || SCPIMM_MODE_DIODE == mode;
	scpimm_context_t* const ctx = SCPIMM_context();
	int16_t err;

	// check correctness of intf->set_mode call
    CU_ASSERT_EQUAL(dm_counters.set_mode, CALLED_ONCE);
    CU_ASSERT_EQUAL(dm_set_mode_last_args.mode, mode);
    if (no_params) {
    	CU_ASSERT_TRUE(dm_set_mode_last_args.params_is_null);
    } else {
    	CU_ASSERT_FALSE(dm_set_mode_last_args.params_is_null);
    }

    // check correctness of current multimeter's mode & params
    CHECK_NO_SCPI_ERROR(scpimm_interface()->get_mode(&cur_mode, &cur_params));
    CU_ASSERT_EQUAL(cur_mode, mode);
    if (!no_params) {
    	ASSERT_EQUAL_BOOL(cur_params.auto_range, dm_set_mode_last_args.params.auto_range);
    }

    // check preset conditions
    CU_ASSERT_EQUAL(ctx->state, SCPIMM_STATE_IDLE);
    CU_ASSERT_EQUAL(ctx->sample_count_num, 1);
    CU_ASSERT_EQUAL(ctx->trigger_count_num, 1);
    CU_ASSERT_EQUAL(ctx->trigger_delay, 0);
    CU_ASSERT_TRUE(ctx->trigger_auto_delay);
    CU_ASSERT_FALSE(ctx->infinite_trigger_count);
    CU_ASSERT_EQUAL(ctx->trigger_src, SCPIMM_TRIG_IMM);
}

/* general checking after CONFIGURE command */
static void check_general_failure() {
	// check correctness of intf->set_mode call
    CU_ASSERT_EQUAL(dm_counters.set_mode, NOT_CALLED);
}

static void configure_with_range(const char* function, const double range, const char* units) {
	receivef("CONFIGURE:%s %0.6g %s", function, range, units);
}

static void configure_with_range_and_res(const char* function, const double range, const char* range_units, const double resolution, const char* resolution_units) {
	receivef("CONFIGURE:%s %0.6g %s,%0.6g %s", function, range, range_units, resolution, resolution_units);
}

static void check_mode_params(const size_t range_index, const scpi_bool_t auto_range, const size_t resolution_index) {
	ASSERT_EQUAL_BOOL(auto_range, dm_set_mode_last_args.params.auto_range);
	CU_ASSERT_EQUAL(range_index, dm_set_mode_last_args.params.range_index);
	CU_ASSERT_EQUAL(resolution_index, dm_set_mode_last_args.params.resolution_index);
}

/* configure function without range/resolution specification */
static void test_configure_no_params(const char* function, const scpimm_mode_t mode) {
	const scpi_bool_t no_params = FALSE;	//	SCPIMM_MODE_CONTINUITY == mode || SCPIMM_MODE_DIODE == mode;

	dm_reset_counters();

	receivef("CONFIGURE:%s\r\n", function);
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	check_general(mode);

	if (!no_params) {
		check_mode_params(MIN_RANGE_INDEX, TRUE, MIN_RESOLUTION_INDEX);
	}
}

static size_t max_index(const double* values) {
	size_t result;

	for (result = 0; values[result] >= 0; result++) {}

	return --result;
}

/* configure function with fixed range and resolution values */
static void test_configure_fix_params(const char* function, scpimm_mode_t mode) {
	const scpi_special_number_t types[] = {SCPI_NUM_MIN, SCPI_NUM_MAX, SCPI_NUM_DEF, SCPI_NUM_AUTO};
	const char* range_values[] = {"MIN", "MAX", "DEF", "AUTO"};
	const char* resolution_values[] = {"MIN", "MAX", "DEF"};
	size_t rangeIndex;
	size_t range_indices[4];
	const double* ranges;
	int16_t err;

	CHECK_NO_SCPI_ERROR(scpimm_interface()->get_allowed_ranges(mode, &ranges, NULL));
	range_indices[0] = MIN_RANGE_INDEX; range_indices[1] = max_index(ranges); range_indices[2] = MIN_RANGE_INDEX; range_indices[3] = MIN_RANGE_INDEX;

	for (rangeIndex = 0; rangeIndex < sizeof(range_values) / sizeof(range_values[0]); ++rangeIndex) {
		size_t resolutionIndex;
		const double* resolutions;
		size_t resolution_indices[3];
		const scpi_bool_t auto_range = types[rangeIndex] == SCPI_NUM_DEF || types[rangeIndex] == SCPI_NUM_AUTO;

		CHECK_NO_SCPI_ERROR(scpimm_interface()->get_allowed_resolutions(mode, range_indices[rangeIndex], &resolutions));
		resolution_indices[0] = MIN_RESOLUTION_INDEX; resolution_indices[1] = max_index(resolutions); resolution_indices[2] = MIN_RESOLUTION_INDEX;

		// CONFIGURE:func <range>
		dm_reset_counters();
		receivef("CONFIGURE:%s %s", function, range_values[rangeIndex]);
		ASSERT_NO_SCPI_ERRORS();
		ASSERT_NO_RESPONSE();
		check_general(mode);
		check_mode_params(range_indices[rangeIndex], auto_range, MIN_RESOLUTION_INDEX);

		for (resolutionIndex = 0; resolutionIndex < sizeof(resolution_values) / sizeof(resolution_values[0]); ++resolutionIndex) {
			// CONFIGURE:func <range>,<resolution>
			dm_reset_counters();
			receivef("CONFIGURE:%s %s,%s", function, range_values[rangeIndex], resolution_values[resolutionIndex]);
			ASSERT_NO_SCPI_ERRORS();
			ASSERT_NO_RESPONSE();
			check_general(mode);
			check_mode_params(range_indices[rangeIndex], auto_range, resolution_indices[resolutionIndex]);
		}
	}
}

static void test_configure_custom_range_and_resolution(const char* function, scpimm_mode_t mode, const double range, const size_t expected_range_index, const double resolution, const size_t expected_resolution_index) {
	// CONFIGURE:func <range>,<resolution>
	dm_reset_counters();
	configure_with_range_and_res(function, range, "", resolution, "");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	check_general(mode);
	check_mode_params(expected_range_index, FALSE, expected_resolution_index);
}

static void test_configure_custom_range(const char* function, scpimm_mode_t mode, const double range, const size_t expected_range_index) {
	int16_t err;
	const double *resolutions;
	size_t resolution_index;

	// CONFIGURE:func <range>
	dm_reset_counters();
	configure_with_range(function, range, "");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	check_general(mode);
	check_mode_params(expected_range_index, FALSE, MIN_RESOLUTION_INDEX);

	CHECK_NO_SCPI_ERROR(scpimm_interface()->get_allowed_resolutions(mode, expected_range_index, &resolutions));

	for (resolution_index = 0; resolutions[resolution_index] >= 0.0; resolution_index++) {
		test_configure_custom_range_and_resolution(function, mode, range, expected_range_index, resolutions[resolution_index], resolution_index);
		test_configure_custom_range_and_resolution(function, mode, range, expected_range_index, 2.0 * resolutions[resolution_index], resolution_index);
	}
}

/* configure function with arbitrary range and resolution values */
static void test_configure_custom_params(const char* function, const scpimm_mode_t mode) {
	int16_t err;
	const double *ranges, *overruns;
	size_t range_index;

	CHECK_NO_SCPI_ERROR(scpimm_interface()->get_allowed_ranges(mode, &ranges, &overruns));

	for (range_index = 0; ranges[range_index] >= 0.0; range_index++) {
		test_configure_custom_range(function, mode, 0.5 * ranges[range_index], range_index);
		test_configure_custom_range(function, mode, ranges[range_index], range_index);
		test_configure_custom_range(function, mode, 0.999 * ranges[range_index] * overruns[range_index], range_index);
	}
}

/* configure with range/resolutions out of range */
static void test_configure_out_of_range(const char* function, scpimm_mode_t mode) {
	int16_t err;
	const double *ranges, *overruns, *resolutions;
	double range, resolution;
	size_t range_index;

	CHECK_NO_SCPI_ERROR(scpimm_interface()->get_allowed_ranges(mode, &ranges, &overruns));

	// range < min range
	range = ranges[0] * 0.5;
	dm_reset_counters();
	configure_with_range(function, range, "");
	ASSERT_NO_SCPI_ERRORS();
	ASSERT_NO_RESPONSE();
	check_general(mode);
	check_mode_params(MIN_RANGE_INDEX, FALSE, MIN_RESOLUTION_INDEX);

	// range > max range
	range = ranges[max_index(ranges)] * overruns[max_index(ranges)] * 1.01;
	dm_reset_counters();
	configure_with_range(function, range, "");
	ASSERT_SCPI_ERROR(SCPI_ERROR_DATA_OUT_OF_RANGE);
	ASSERT_NO_RESPONSE();
	check_general_failure();
	clearscpi_errors();

	for (range_index = 0; ranges[range_index] >= 0.0; range_index++) {
		range = ranges[range_index];
		CHECK_NO_SCPI_ERROR(scpimm_interface()->get_allowed_resolutions(mode, range_index, &resolutions));

		// resolution < min resolution
		resolution = resolutions[0] * 0.99;
		dm_reset_counters();
		configure_with_range_and_res(function, range, "", resolution, "");
		ASSERT_SCPI_ERROR(SCPI_ERROR_CANNOT_ACHIEVE_REQUESTED_RESOLUTION);
		ASSERT_NO_RESPONSE();
		check_general_failure();
		clearscpi_errors();

		// resolution > max resolution
		resolution = resolutions[max_index(resolutions)] * 2.0;
		dm_reset_counters();
		configure_with_range_and_res(function, range, "", resolution, "");
		ASSERT_NO_SCPI_ERRORS();
		ASSERT_NO_RESPONSE();
		check_general(mode);
		check_mode_params(range_index, FALSE, max_index(resolutions));
	}
}

static void test_configure_units(const char* function, scpimm_mode_t mode, const char* prefs[], const double mults[]) {
	int16_t err;
	const double *ranges;
	size_t range_index;

	CHECK_NO_SCPI_ERROR(scpimm_interface()->get_allowed_ranges(mode, &ranges, NULL));

	for (range_index = 0; ranges[range_index] >= 0.0; range_index++) {
		const double range = ranges[range_index] * 0.5;
		const double *resolutions;
		size_t resolution_index, unit_index;

		CHECK_NO_SCPI_ERROR(scpimm_interface()->get_allowed_resolutions(mode, range_index, &resolutions));

		for (unit_index = 0; prefs[unit_index] != NULL; ++unit_index) {
			dm_reset_counters();
			configure_with_range(function, range / mults[unit_index], prefs[unit_index]);
			ASSERT_NO_SCPI_ERRORS();
			ASSERT_NO_RESPONSE();
			check_general(mode);
			check_mode_params(range_index, FALSE, MIN_RESOLUTION_INDEX);
		}

		for (resolution_index = 0; resolutions[resolution_index] >= 0.0; resolution_index++) {
			const double resolution = resolutions[resolution_index] * 2.0;

			for (unit_index = 0; prefs[unit_index] != NULL; ++unit_index) {
				dm_reset_counters();
				configure_with_range_and_res(function, range / mults[range_index], prefs[range_index], resolution / mults[range_index], prefs[range_index]);
				ASSERT_NO_SCPI_ERRORS();
				ASSERT_NO_RESPONSE();
				check_general(mode);
				check_mode_params(range_index, FALSE, resolution_index);
			}
		}
	}
}

static void test_impl(const char* function, scpimm_mode_t mode, const char* prefs[], const double mults[]) {
	test_configure_no_params(function, mode);
	test_configure_fix_params(function, mode);
	test_configure_custom_params(function, mode);
	test_configure_out_of_range(function, mode);
	test_configure_units(function, mode, prefs, mults);
}

static void test_configureQ_impl(const char* function, scpimm_mode_t mode, const char* mode_name) {
	int16_t err;
	const scpi_bool_t no_params = FALSE;	//	SCPIMM_MODE_CONTINUITY == mode || SCPIMM_MODE_DIODE == mode;
	const double *ranges;
	size_t range_index;
	char buf[100];

	if (no_params) {
		dm_reset_counters();
		receivef("CONFIGURE:%s\r\n", function);
		receivef("CONFIGURE?\r\n");
		ASSERT_NO_SCPI_ERRORS();
		sprintf(buf, "\"%s\"\r\n", mode_name);
		ASSERT_RESPONSE(buf);
		return;
	}

	CHECK_NO_SCPI_ERROR(scpimm_interface()->get_allowed_ranges(mode, &ranges, NULL));

	for (range_index = 0; ranges[range_index] >= 0.0; range_index++) {
		const double range = ranges[range_index] * 0.5;
		const double *resolutions;
		size_t resolution_index;
		double actual_range, actual_resolution;
		const char* result;

		CHECK_NO_SCPI_ERROR(scpimm_interface()->get_allowed_resolutions(mode, range_index, &resolutions));

		dm_reset_counters();
		configure_with_range(function, ranges[range_index], "");
		receivef("CONFIGURE?\r\n");
		ASSERT_NO_SCPI_ERRORS();
		sprintf(buf, "\"%s ", mode_name);
		result = dm_read_entire_output_buffer();
		CU_ASSERT_EQUAL(strncmp(buf, result, strlen(buf)), 0);
		sscanf(result + strlen(buf), "%le,%le", &actual_range, &actual_resolution);
		ASSERT_DOUBLE_EQUAL(actual_range, ranges[range_index]);
		ASSERT_DOUBLE_EQUAL(actual_resolution, resolutions[MIN_RESOLUTION_INDEX]);

		for (resolution_index = 0; resolutions[resolution_index] >= 0.0; resolution_index++) {
			const double resolution = resolutions[resolution_index] * 2.0;

			dm_reset_counters();
			configure_with_range_and_res(function, range, "", resolution, "");
			receivef("CONFIGURE?\r\n");
			ASSERT_NO_SCPI_ERRORS();
			sprintf(buf, "\"%s ", mode_name);
			result = dm_read_entire_output_buffer();
			CU_ASSERT_EQUAL(strncmp(buf, result, strlen(buf)), 0);
			sscanf(result + strlen(buf), "%le,%le", &actual_range, &actual_resolution);
			ASSERT_DOUBLE_EQUAL(actual_range, ranges[range_index]);
			ASSERT_DOUBLE_EQUAL(actual_resolution, resolutions[resolution_index]);
		}
	}
}

static void test_configure_voltage_dc() {
	test_impl("VOLTAGE", SCPIMM_MODE_DCV, voltage_prefixes, voltage_mults);
	test_impl("VOLTAGE:DC", SCPIMM_MODE_DCV, voltage_prefixes, voltage_mults);
}

static void test_configure_voltage_dc_ratio() {
	test_impl("VOLTAGE:DC:RATIO", SCPIMM_MODE_DCV_RATIO, voltage_prefixes, voltage_mults);
}

static void test_configure_voltage_ac() {
	test_impl("VOLTAGE:AC", SCPIMM_MODE_ACV, voltage_prefixes, voltage_mults);
}

static void test_configure_current_dc() {
	test_impl("CURRENT", SCPIMM_MODE_DCC, current_prefixes, current_mults);
	test_impl("CURRENT:DC", SCPIMM_MODE_DCC, current_prefixes, current_mults);
}

static void test_configure_current_ac() {
	test_impl("CURRENT:AC", SCPIMM_MODE_ACC, current_prefixes, current_mults);
}

static void test_configure_resistance() {
	test_impl("RESISTANCE", SCPIMM_MODE_RESISTANCE_2W, resistance_prefixes, resistance_mults);
}

static void test_configure_fresistance() {
	test_impl("FRESISTANCE", SCPIMM_MODE_RESISTANCE_4W, resistance_prefixes, resistance_mults);
}

static void test_configureQ() {
	test_configureQ_impl("VOLTAGE", SCPIMM_MODE_DCV, "VOLT");
	test_configureQ_impl("VOLTAGE:DC", SCPIMM_MODE_DCV, "VOLT");
	test_configureQ_impl("VOLTAGE:DC:RATIO", SCPIMM_MODE_DCV_RATIO, "VOLT:DC:RAT");
	test_configureQ_impl("VOLTAGE:AC", SCPIMM_MODE_ACV, "VOLT:AC");
	test_configureQ_impl("CURRENT", SCPIMM_MODE_DCC, "CURR");
	test_configureQ_impl("CURRENT:DC", SCPIMM_MODE_DCC, "CURR");
	test_configureQ_impl("CURRENT:AC", SCPIMM_MODE_ACC, "CURR:AC");
	test_configureQ_impl("RESISTANCE", SCPIMM_MODE_RESISTANCE_2W, "RES");
	test_configureQ_impl("FRESISTANCE", SCPIMM_MODE_RESISTANCE_4W, "FRES");
}

int main() {
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    ADD_SUITE("CONFIGURE");

    /* Add the tests to the suite */
    ADD_TEST(test_configure_voltage_dc);
    ADD_TEST(test_configure_voltage_dc_ratio);
    ADD_TEST(test_configure_voltage_ac);
    ADD_TEST(test_configure_current_dc);
    ADD_TEST(test_configure_current_ac);
    ADD_TEST(test_configure_resistance);
    ADD_TEST(test_configure_fresistance);
    ADD_TEST(test_configureQ);

    /* Run all tests using the CUnit Basic interface */
    return RUN_ALL_TESTS();
}

