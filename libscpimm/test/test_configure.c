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

static void configure_with_range(const char* function, const double range) {
	receivef("CONFIGURE:%s %0.3g", function, range);
}

static void configure_with_range_and_res(const char* function, const double range, const double resolution) {
	receivef("CONFIGURE:%s %0.3g,%0.3g", function, range, resolution);
}

static void check_mode_params(const double range, const bool_t auto_range, const double resolution) {
	ASSERT_EQUAL_BOOL(auto_range, dm_set_mode_last_args.params.auto_range);
	ASSERT_DOUBLE_EQUAL(range, dm_set_mode_last_args.params.range);
	ASSERT_DOUBLE_EQUAL(resolution, dm_set_mode_last_args.params.resolution);
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
		double min_range, min_resolution;

		ASSERT_NO_SCPI_ERROR(scpimm_interface()->get_possible_range(mode, &min_range, NULL));
		ASSERT_NO_SCPI_ERROR(scpimm_interface()->get_possible_resolution(mode, min_range, &min_resolution, NULL));
		check_mode_params(min_range, TRUE, min_resolution);
	}
}

/* configure function with fixed range and resolution values */
static void test_configure_fix_params(const char* function, scpimm_mode_t mode) {
	const scpi_special_number_t types[] = {SCPI_NUM_MIN, SCPI_NUM_MAX, SCPI_NUM_DEF};
	const char* strs[] = {"MIN", "MAX", "DEF"};
	size_t rangeIndex;
	double min_range, max_range;
	double ranges[3];

	reset();

	ASSERT_NO_SCPI_ERROR(scpimm_interface()->get_possible_range(mode, &min_range, &max_range));
	ranges[0] = min_range; ranges[1] = max_range; ranges[2] = min_range;

	for (rangeIndex = 0; rangeIndex < sizeof(types) / sizeof(types[0]); ++rangeIndex) {
		size_t resolutionIndex;
		double min_resolution, max_resolution;
		double resolutions[3];

		ASSERT_NO_SCPI_ERROR(scpimm_interface()->get_possible_resolution(mode, ranges[rangeIndex], &min_resolution, &max_resolution));
		resolutions[0] = min_resolution; resolutions[1] = max_resolution; resolutions[2] = min_resolution;

		// CONFIGURE:func <range>
		dm_reset_counters();
		receivef("CONFIGURE:%s %s", function, strs[rangeIndex]);
		assert_no_scpi_errors();
		asset_no_data();
		check_general(mode);
		check_mode_params(ranges[rangeIndex], types[rangeIndex] == SCPI_NUM_DEF, min_resolution);

		for (resolutionIndex = 0; resolutionIndex < sizeof(types) / sizeof(types[0]); ++resolutionIndex) {
			// CONFIGURE:func <range>,<resolution>
			dm_reset_counters();
			receivef("CONFIGURE:%s %s,%s", function, strs[rangeIndex], strs[resolutionIndex]);
			assert_no_scpi_errors();
			asset_no_data();
			check_general(mode);
			check_mode_params(ranges[rangeIndex], types[rangeIndex] == SCPI_NUM_DEF, resolutions[resolutionIndex]);
		}
	}
}

/* configure function with arbitrary range and resolution values */
static void test_configure_custom_params(const char* function, scpimm_mode_t mode) {
	double range, min_range, max_range;

	reset();

	ASSERT_NO_SCPI_ERROR(scpimm_interface()->get_possible_range(mode, &min_range, &max_range));

	for (range = min_range; range <= max_range * (1 + FLOAT_DELTA); range *= 10.0) {
		double resolution, min_resolution, max_resolution;

		ASSERT_NO_SCPI_ERROR(scpimm_interface()->get_possible_resolution(mode, range, &min_resolution, &max_resolution));

		// CONFIGURE:func <range>
		dm_reset_counters();
		configure_with_range(function, range);
		assert_no_scpi_errors();
		asset_no_data();
		check_general(mode);
		check_mode_params(range, FALSE, min_resolution);

		for (resolution = min_resolution; resolution <= max_resolution * (1 + FLOAT_DELTA); resolution *= 10.0) {
			// CONFIGURE:func <range>,<resolution>
			dm_reset_counters();
			configure_with_range_and_res(function, range, resolution);
			assert_no_scpi_errors();
			asset_no_data();
			check_general(mode);
			check_mode_params(range, FALSE, resolution);
		}
	}

}

/* configure with range/resolutions out of range */
static void test_configure_out_of_range(const char* function, scpimm_mode_t mode) {
	double range, min_range, max_range;
	double resolution, min_resolution, max_resolution;

	reset();

	ASSERT_NO_SCPI_ERROR(scpimm_interface()->get_possible_range(mode, &min_range, &max_range));

	// range < min range
	range = min_range * RANGE_UNDERFLOW;
	ASSERT_NO_SCPI_ERROR(scpimm_interface()->get_possible_resolution(mode, min_range, &min_resolution, &max_resolution));
	dm_reset_counters();
	configure_with_range(function, range);
	assert_no_scpi_errors();
	asset_no_data();
	check_general(mode);
	check_mode_params(min_range, FALSE, min_resolution);

	// range > max range
	range = max_range * RANGE_OVERFLOW;
	dm_reset_counters();
	configure_with_range(function, range);
	assert_scpi_error(SCPI_ERROR_DATA_OUT_OF_RANGE);
	asset_no_data();
	check_general_failure();
	clearscpi_errors();

	range = min_range;
	ASSERT_NO_SCPI_ERROR(scpimm_interface()->get_possible_resolution(mode, range, &min_resolution, &max_resolution));

	// resolution < min resolution
	resolution = min_resolution * RESOLUTION_UNDERFLOW;
	dm_reset_counters();
	configure_with_range_and_res(function, range, resolution);
	assert_scpi_error(SCPI_ERROR_CANNOT_ACHIEVE_REQUESTED_RESOLUTION);
	asset_no_data();
	check_general_failure();
	clearscpi_errors();

	// resolution > max resolution
	resolution = max_resolution * RESOLUTION_OVERFLOW;
	dm_reset_counters();
	configure_with_range_and_res(function, range, resolution);
	assert_no_scpi_errors();
	asset_no_data();
	check_general(mode);
	check_mode_params(range, FALSE, max_resolution);
}

#if ddd
static void test_configure_units(const char* function, scpimm_mode_t mode, const char* units) {
	const float range = 1.0;
	const float resolution = 0.1;
	const char* const prefs[] = {"", "k", "K", "m", "M", "u", "U"};
	const float const gains[] = {1.0, 1.0e3, 1.0e3, 1.0e-3, 1.0e-3, 1.0e-6, 1.0e-6};
	size_t index;

	reset();

	for (index = 0; index < sizeof(prefs) / sizeof(prefs[0]); ++index) {
		reset_counters();
		receivef("CONFIGURE:%s %f %s%s,%f %s%s", function, (double) range, prefs[index], units, (double) resolution, prefs[index], units);
		assert_no_scpi_errors();
		asset_no_data();
		check_last_mode(mode, range * gains[index], resolution * gains[index]);

		reset_counters();
		receivef("CONFIGURE:%s %f%s%s,%f%s%s", function, (double) range, prefs[index], units, (double) resolution, prefs[index], units);
		assert_no_scpi_errors();
		asset_no_data();
		check_last_mode(mode, range * gains[index], resolution * gains[index]);

		/* TODO check invalid parameters errors */
	}
}
#endif

static void test_impl(const char* function, scpimm_mode_t mode, const char* units) {
	test_configure_no_params(function, mode);
	if (SCPIMM_MODE_CONTINUITY != mode && SCPIMM_MODE_DIODE != mode) {
		test_configure_fix_params(function, mode);
		test_configure_custom_params(function, mode);
		test_configure_out_of_range(function, mode);
		//test_configure_units(function, mode, units);
	}
}

int init_suite(void) {
    return 0;
}

int clean_suite(void) {
    return 0;
}

void test_configure_voltage_dc() {
	test_impl("VOLTAGE", SCPIMM_MODE_DCV, "V");
	test_impl("VOLTAGE:DC", SCPIMM_MODE_DCV, "V");
}

void test_configure_voltage_dc_ratio() {
	test_impl("VOLTAGE:DC:RATIO", SCPIMM_MODE_DCV_RATIO, "V");
}

void test_configure_voltage_ac() {
	test_impl("VOLTAGE:AC", SCPIMM_MODE_ACV, "V");
}

void test_configure_current_dc() {
	test_impl("CURRENT", SCPIMM_MODE_DCC, "A");
	test_impl("CURRENT:DC", SCPIMM_MODE_DCC, "A");
}

void test_configure_current_ac() {
	test_impl("CURRENT:AC", SCPIMM_MODE_ACC, "A");
}

void test_configure_resistance() {
	test_impl("RESISTANCE", SCPIMM_MODE_RESISTANCE_2W, "Ohm");
}

void test_configure_fresistance() {
	test_impl("FRESISTANCE", SCPIMM_MODE_RESISTANCE_4W, "Ohm");
}

void test_configure_frequency() {
	test_impl("FREQUENCY", SCPIMM_MODE_FREQUENCY, "Hz");
}

void test_configure_period() {
	test_impl("PERIOD", SCPIMM_MODE_PERIOD, "S");
}

void test_configure_continuity() {
	test_impl("CONTINUITY", SCPIMM_MODE_CONTINUITY, NULL);
}

void test_configure_diode() {
	test_impl("DIODE", SCPIMM_MODE_DIODE, NULL);
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
    if ((NULL == CU_add_test(pSuite, "test configure:voltage:dc", test_configure_voltage_dc))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test configure:voltage:dc:ratio", test_configure_voltage_dc_ratio))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test configure:voltage:ac", test_configure_voltage_ac))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test configure:current:dc", test_configure_current_dc))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test configure:current:ac", test_configure_current_ac))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test configure:resistance", test_configure_resistance))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test configure:fresistance", test_configure_fresistance))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test configure:frequency", test_configure_frequency))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test configure:period", test_configure_period))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test configure:continuity", test_configure_continuity))) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "test configure:diode", test_configure_diode))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

