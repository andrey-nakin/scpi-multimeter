#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "test_utils.h"
#include "default_multimeter.h"

#define	RANGE_UNDERFLOW	0.90
#define	RANGE_OVERFLOW 1.10
#define	RESOLUTION_UNDERFLOW RANGE_UNDERFLOW
#define	RESOLUTION_OVERFLOW RANGE_OVERFLOW

static const char* voltage_prefixes[] = {"V", "kV", "KV", "mV", "MV", "uV", "UV", NULL};
static const double voltage_mults[] = {1, 1e3, 1e3, 1e-3, 1e-3, 1e-6, 1e-6, 0.0};
static const char* current_prefixes[] = {"A", "kA", "KA", "mA", "MA", "uA", "UA", NULL};
static const double current_mults[] = {1, 1e3, 1e3, 1e-3, 1e-3, 1e-6, 1e-6, 0.0};
static const char* resistance_prefixes[] = {"Ohm", "kOhm", "KOhm", "mOhm", "MOhm", NULL};
static const double resistance_mults[] = {1, 1e3, 1e3, 1e6, 1e6, 0.0};
static const char* frequency_prefixes[] = {"Hz", "kHz", "KHz", "mHz", "MHz", "gHz", "GHz", NULL};
static const double frequency_mults[] = {1, 1e3, 1e3, 1e6, 1e6, 1e9, 1e9, 0.0};
static const char* time_prefixes[] = {"s", "ps", "Ps",   "ns", "Ns", "us", "Us", "ms", "Ms", "min", "hr", NULL};
static const double time_mults[] =   {1,   1e-12, 1e-12, 1e-9, 1e-9, 1e-6, 1e-6, 1e-3, 1e-3, 60,    3600, 0};

static void reset() {
	init_scpimm();
	clearscpi_errors();
	init_test_vars();
}

/* general checking after CONFIGURE command */
static void check_general(const scpimm_mode_t mode) {
	scpimm_mode_t cur_mode;
	scpimm_mode_params_t cur_params;
	const bool_t no_params = SCPIMM_MODE_CONTINUITY == mode || SCPIMM_MODE_DIODE == mode;
	scpimm_context_t* const ctx = SCPIMM_context();

	// check correctness of intf->set_mode call
    CU_ASSERT_EQUAL(dm_counters.set_mode, CALLED_ONCE);
    CU_ASSERT_EQUAL(dm_set_mode_last_args.mode, mode);
    if (no_params) {
    	CU_ASSERT_TRUE(dm_set_mode_last_args.params_is_null);
    } else {
    	CU_ASSERT_FALSE(dm_set_mode_last_args.params_is_null);
    }

    // check correctness of current multimeter's mode & params
    ASSERT_NO_SCPI_ERROR(scpimm_interface()->get_mode(&cur_mode, &cur_params));
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

static void check_mode_params(const size_t range_index, const bool_t auto_range, const size_t resolution_index) {
	ASSERT_EQUAL_BOOL(auto_range, dm_set_mode_last_args.params.auto_range);
	CU_ASSERT_EQUAL(range_index, dm_set_mode_last_args.params.range_index);
	CU_ASSERT_EQUAL(resolution_index, dm_set_mode_last_args.params.resolution_index);
}

/* configure function without range/resolution specification */
static void test_configure_no_params(const char* function, const scpimm_mode_t mode) {
	const bool_t no_params = SCPIMM_MODE_CONTINUITY == mode || SCPIMM_MODE_DIODE == mode;

	reset();
	dm_reset_counters();

	receivef("CONFIGURE:%s\r\n", function);
	assert_no_scpi_errors();
	asset_no_data();
	check_general(mode);

	if (!no_params) {
		check_mode_params(0, TRUE, 0);
	}
}

static size_t max_index(const double* values) {
	size_t result;

	for (result = 0; values[result] >= 0; result++) {}

	return --result;
}

/* configure function with fixed range and resolution values */
static void test_configure_fix_params(const char* function, scpimm_mode_t mode) {
	const scpi_special_number_t types[] = {SCPI_NUM_MIN, SCPI_NUM_MAX, SCPI_NUM_DEF};
	const char* strs[] = {"MIN", "MAX", "DEF"};
	size_t rangeIndex;
	size_t range_indices[3];
	const double* ranges;

	reset();

	ASSERT_NO_SCPI_ERROR(scpimm_interface()->get_allowed_ranges(mode, &ranges, NULL));
	range_indices[0] = 0; range_indices[1] = max_index(ranges); range_indices[2] = 0;

	for (rangeIndex = 0; rangeIndex < sizeof(types) / sizeof(types[0]); ++rangeIndex) {
		size_t resolutionIndex;
		const double* resolutions;
		size_t resolution_indices[3];

		ASSERT_NO_SCPI_ERROR(scpimm_interface()->get_allowed_resolutions(mode, ranges[range_indices[rangeIndex]], &resolutions));
		resolution_indices[0] = 0; resolution_indices[1] = max_index(resolutions); resolution_indices[2] = 0;

		// CONFIGURE:func <range>
		dm_reset_counters();
		receivef("CONFIGURE:%s %s", function, strs[rangeIndex]);
		assert_no_scpi_errors();
		asset_no_data();
		check_general(mode);
		check_mode_params(range_indices[rangeIndex], types[rangeIndex] == SCPI_NUM_DEF, 0);

		for (resolutionIndex = 0; resolutionIndex < sizeof(types) / sizeof(types[0]); ++resolutionIndex) {
			// CONFIGURE:func <range>,<resolution>
			dm_reset_counters();
			receivef("CONFIGURE:%s %s,%s", function, strs[rangeIndex], strs[resolutionIndex]);
			assert_no_scpi_errors();
			asset_no_data();
			check_general(mode);
			check_mode_params(range_indices[rangeIndex], types[rangeIndex] == SCPI_NUM_DEF, resolution_indices[resolutionIndex]);
		}
	}
}

/* configure function with arbitrary range and resolution values */
static void test_configure_custom_params(const char* function, scpimm_mode_t mode) {
	double range, min_range, max_range;
#ifdef aaa
	reset();

	ASSERT_NO_SCPI_ERROR(scpimm_interface()->get_possible_range(mode, &min_range, &max_range));

	for (range = min_range; range <= max_range * (1 + FLOAT_DELTA); range *= 10.0) {
		double resolution, min_resolution, max_resolution;

		ASSERT_NO_SCPI_ERROR(scpimm_interface()->get_possible_resolution(mode, range, &min_resolution, &max_resolution));

		// CONFIGURE:func <range>
		dm_reset_counters();
		configure_with_range(function, range, "");
		assert_no_scpi_errors();
		asset_no_data();
		check_general(mode);
		check_mode_params(range, FALSE, min_resolution);

		for (resolution = min_resolution; resolution <= max_resolution * (1 + FLOAT_DELTA); resolution *= 10.0) {
			// CONFIGURE:func <range>,<resolution>
			dm_reset_counters();
			configure_with_range_and_res(function, range, "", resolution, "");
			assert_no_scpi_errors();
			asset_no_data();
			check_general(mode);
			check_mode_params(range, FALSE, resolution);
		}
	}
#endif
}

/* configure with range/resolutions out of range */
static void test_configure_out_of_range(const char* function, scpimm_mode_t mode) {
	double range, min_range, max_range;
	double resolution, min_resolution, max_resolution;
#ifdef aaa
	reset();

	ASSERT_NO_SCPI_ERROR(scpimm_interface()->get_possible_range(mode, &min_range, &max_range));

	// range < min range
	range = min_range * RANGE_UNDERFLOW;
	ASSERT_NO_SCPI_ERROR(scpimm_interface()->get_possible_resolution(mode, min_range, &min_resolution, &max_resolution));
	dm_reset_counters();
	configure_with_range(function, range, "");
	assert_no_scpi_errors();
	asset_no_data();
	check_general(mode);
	check_mode_params(min_range, FALSE, min_resolution);

	// range > max range
	range = max_range * RANGE_OVERFLOW;
	dm_reset_counters();
	configure_with_range(function, range, "");
	assert_scpi_error(SCPI_ERROR_DATA_OUT_OF_RANGE);
	asset_no_data();
	check_general_failure();
	clearscpi_errors();

	range = min_range;
	ASSERT_NO_SCPI_ERROR(scpimm_interface()->get_possible_resolution(mode, range, &min_resolution, &max_resolution));

	// resolution < min resolution
	resolution = min_resolution * RESOLUTION_UNDERFLOW;
	dm_reset_counters();
	configure_with_range_and_res(function, range, "", resolution, "");
	assert_scpi_error(SCPI_ERROR_CANNOT_ACHIEVE_REQUESTED_RESOLUTION);
	asset_no_data();
	check_general_failure();
	clearscpi_errors();

	// resolution > max resolution
	resolution = max_resolution * RESOLUTION_OVERFLOW;
	dm_reset_counters();
	configure_with_range_and_res(function, range, "", resolution, "");
	assert_no_scpi_errors();
	asset_no_data();
	check_general(mode);
	check_mode_params(range, FALSE, max_resolution);
#endif
}

static void test_configure_units(const char* function, scpimm_mode_t mode, const char* prefs[], const double mults[]) {
#ifdef aaa
	double range, min_range;
	double resolution, min_resolution;
	size_t range_index;

	reset();

	ASSERT_NO_SCPI_ERROR(scpimm_interface()->get_possible_range(mode, &min_range, NULL));
	range = min_range;
	ASSERT_NO_SCPI_ERROR(scpimm_interface()->get_possible_resolution(mode, range, &min_resolution, NULL));
	resolution = min_resolution;

	for (range_index = 0; prefs[range_index] != NULL; ++range_index) {
		size_t resolution_index;

		dm_reset_counters();
		configure_with_range(function, range / mults[range_index], prefs[range_index]);
		assert_no_scpi_errors();
		asset_no_data();
		check_general(mode);
		check_mode_params(range, FALSE, min_resolution);

		for (resolution_index = 0; prefs[resolution_index] != NULL; ++resolution_index) {
			dm_reset_counters();
			configure_with_range_and_res(function, range / mults[range_index], prefs[range_index], resolution / mults[range_index], prefs[range_index]);
			assert_no_scpi_errors();
			asset_no_data();
			check_general(mode);
			check_mode_params(range, FALSE, resolution);
		}
	}
#endif
}

static void test_impl(const char* function, scpimm_mode_t mode, const char* prefs[], const double mults[]) {
	test_configure_no_params(function, mode);
	if (SCPIMM_MODE_CONTINUITY != mode && SCPIMM_MODE_DIODE != mode) {
		test_configure_fix_params(function, mode);
		test_configure_custom_params(function, mode);
		test_configure_out_of_range(function, mode);
		test_configure_units(function, mode, prefs, mults);
	}
}

static void test_configureQ_impl(const char* function, scpimm_mode_t mode, const char* mode_name) {
	const bool_t no_params = SCPIMM_MODE_CONTINUITY == mode || SCPIMM_MODE_DIODE == mode;
	char buf[100], *result = dm_output_buffer();
	double range, resolution;

	reset();
	dm_reset_counters();

	receivef("CONFIGURE:%s\r\n", function);
	receivef("CONFIGURE?\r\n");
	assert_no_scpi_errors();

	if (!no_params) {
		sprintf(buf, "\"%s ", mode_name);
		CU_ASSERT_EQUAL(strncmp(buf, result, strlen(buf)), 0);
		sscanf(buf + strlen(buf), "%g,%g", &range, &resolution);
	} else {
		sprintf(buf, "\"%s\"\r\n", mode_name);
		CU_ASSERT_STRING_EQUAL(buf, result);
	}
}

int init_suite(void) {
    return 0;
}

int clean_suite(void) {
    return 0;
}

void test_configure_voltage_dc() {
	test_impl("VOLTAGE", SCPIMM_MODE_DCV, voltage_prefixes, voltage_mults);
	test_impl("VOLTAGE:DC", SCPIMM_MODE_DCV, voltage_prefixes, voltage_mults);
}

void test_configure_voltage_dc_ratio() {
	test_impl("VOLTAGE:DC:RATIO", SCPIMM_MODE_DCV_RATIO, voltage_prefixes, voltage_mults);
}

void test_configure_voltage_ac() {
	test_impl("VOLTAGE:AC", SCPIMM_MODE_ACV, voltage_prefixes, voltage_mults);
}

void test_configure_current_dc() {
	test_impl("CURRENT", SCPIMM_MODE_DCC, current_prefixes, current_mults);
	test_impl("CURRENT:DC", SCPIMM_MODE_DCC, current_prefixes, current_mults);
}

void test_configure_current_ac() {
	test_impl("CURRENT:AC", SCPIMM_MODE_ACC, current_prefixes, current_mults);
}

void test_configure_resistance() {
	test_impl("RESISTANCE", SCPIMM_MODE_RESISTANCE_2W, resistance_prefixes, resistance_mults);
}

void test_configure_fresistance() {
	test_impl("FRESISTANCE", SCPIMM_MODE_RESISTANCE_4W, resistance_prefixes, resistance_mults);
}

void test_configure_frequency() {
	test_impl("FREQUENCY", SCPIMM_MODE_FREQUENCY, frequency_prefixes, frequency_mults);
}

void test_configure_period() {
	test_impl("PERIOD", SCPIMM_MODE_PERIOD, time_prefixes, time_mults);
}

void test_configure_continuity() {
	test_impl("CONTINUITY", SCPIMM_MODE_CONTINUITY, NULL, NULL);
}

void test_configure_diode() {
	test_impl("DIODE", SCPIMM_MODE_DIODE, NULL, NULL);
}

void test_configureQ() {
	test_configureQ_impl("VOLTAGE", SCPIMM_MODE_DCV, "VOLT");
	test_configureQ_impl("VOLTAGE:DC", SCPIMM_MODE_DCV, "VOLT");
}

int main() {
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    pSuite = CU_add_suite("CONFIGURE", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "configure:voltage:dc", test_configure_voltage_dc))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "configure:voltage:dc:ratio", test_configure_voltage_dc_ratio))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "configure:voltage:ac", test_configure_voltage_ac))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "configure:current:dc", test_configure_current_dc))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "configure:current:ac", test_configure_current_ac))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "configure:resistance", test_configure_resistance))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "configure:fresistance", test_configure_fresistance))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "configure:frequency", test_configure_frequency))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "configure:period", test_configure_period))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "configure:continuity", test_configure_continuity))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "configure:diode", test_configure_diode))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "configure?", test_configureQ))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

