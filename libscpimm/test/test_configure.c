#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "test_utils.h"

static unsigned mode_counter, params_counter;
static scpimm_mode_t last_mode, last_params_mode;
static scpi_number_t last_range;
static scpi_number_t last_resolution;
static bool_t last_auto_range, last_no_params;

static scpimm_mode_t actual_mode;
static double actual_range, actual_resolution;
static bool_t actual_auto_range;

/**
 * Function emulates real multimeter. It attempts to establish desired measurement mode, say VOLT:DC, with given parameters
 */
static int16_t set_mode(scpimm_mode_t mode, const scpimm_mode_params_t* const params) {
	double new_range, new_resolution;
	bool_t new_auto_range;

	last_mode = mode;
	++mode_counter;

	if (params) {
		last_no_params = FALSE;

		++params_counter;
		last_params_mode = mode;
		last_range = params->range;
		last_auto_range = params->auto_range;
		last_resolution = params->resolution;

		switch (params->range.type) {
		case SCPI_NUM_NUMBER:
			if (params->range.value < MIN_RANGE) {
				new_range = MIN_RANGE;
			} else if (params->range.value > MAX_RANGE * INCREASE_DELTA) {
				return SCPI_ERROR_DATA_OUT_OF_RANGE;
			} else {
				new_range = params->range.value;
			}
			break;

		case SCPI_NUM_MIN:
			new_range = MIN_RANGE;
			break;

		case SCPI_NUM_MAX:
			new_range = MAX_RANGE;
			break;

		case SCPI_NUM_DEF:
			new_range = MIN_RANGE;
			break;

		default:
			return SCPI_ERROR_ILLEGAL_PARAMETER_VALUE;
		}

		switch (params->resolution.type) {
		case SCPI_NUM_NUMBER:
			if (params->resolution.value > new_range * WORST_RESOLUTION) {
				new_resolution = new_range * WORST_RESOLUTION;
			} else if (params->resolution.value < new_range * BEST_RESOLUTION * DECREASE_DELTA) {
				return SCPI_ERROR_CANNOT_ACHIEVE_REQUESTED_RESOLUTION;
			} else {
				new_resolution = params->resolution.value;
			}
			break;

		case SCPI_NUM_DEF:
		case SCPI_NUM_MIN:
			new_resolution = new_range * BEST_RESOLUTION;
			break;

		case SCPI_NUM_MAX:
			new_resolution = new_range * WORST_RESOLUTION;
			break;

		default:
			return SCPI_ERROR_ILLEGAL_PARAMETER_VALUE;
		}

		new_auto_range = params->auto_range;
	} else {	//	if (params)
		last_no_params = TRUE;
	}	//	if (params)

	actual_mode = last_mode;
	actual_range = new_range;
	actual_auto_range = new_auto_range;
	actual_resolution = new_resolution;

	return SCPI_ERROR_OK;
}

static void reset_counters() {
	mode_counter = 0;
	params_counter = 0;
}

static void reset() {
	init_scpimm();
	clearscpi_errors();
	init_test_vars();
	SCPIMM_context()->interface->set_mode = set_mode;

	reset_counters();
	last_mode = 0;
	last_params_mode = 0;
	last_range.type = 0;
	last_resolution.type = 0;
	last_auto_range = (last_auto_range) -1;
}

static void check_last_mode(scpimm_mode_t mode, const scpi_number_t* range, const bool_t* auto_range, const scpi_number_t* resolution) {
	const scpimm_context_t* const ctx = SCPIMM_context();
	const scpimm_mode_params_t* const params = SCPIMM_mode_params(ctx, mode);

    CU_ASSERT_EQUAL(mode_counter, 1);
    CU_ASSERT_EQUAL(last_mode, mode);
	
	if (params) {
	    CU_ASSERT_EQUAL(params_counter, 1);
	    CU_ASSERT_EQUAL(last_params_mode, mode);

	    if (range) {
	    	assert_number_equals(&last_range, range);
	    	assert_number_equals(&params->range, range);
	    }

	    if (auto_range) {
	    	assert_bool_equals(last_auto_range, *auto_range);
	    	assert_bool_equals(params->auto_range, *auto_range);
	    }

		if (resolution) {
			assert_number_equals(&last_resolution, resolution);
			assert_number_equals(&params->resolution, resolution);
		}
	}
}

/* configure function with default range and resolution */
static void test_configure_no_params(const char* function, scpimm_mode_t mode) {
	scpi_number_t* rangeVar = range_context_var(mode);
	scpi_number_t* resolutionVar = resolution_context_var(mode);
	const scpi_number_t def = {0.0, SCPI_UNIT_NONE, SCPI_NUM_DEF};

	reset();

	if (rangeVar && resolutionVar) {
		const scpi_number_t customRange = {1.0, SCPI_UNIT_NONE, SCPI_NUM_NUMBER};
		const scpi_number_t customResolution = {0.1, SCPI_UNIT_NONE, SCPI_NUM_NUMBER};
		*rangeVar = customRange;
		*resolutionVar = customResolution;
	}

	receivef("CONFIGURE:%s\r\n", function);
	assert_no_scpi_errors();
	asset_no_data();
	check_last_mode(mode, &def, NULL, &def);

}

/* configure function with default range and resolution */
static void test_configure_fix_params(const char* function, scpimm_mode_t mode) {
	const scpi_number_t numbers[] = {
		{0.0, SCPI_UNIT_NONE, SCPI_NUM_MIN}, 
		{0.0, SCPI_UNIT_NONE, SCPI_NUM_MAX}, 
		{0.0, SCPI_UNIT_NONE, SCPI_NUM_DEF}
	};
	const char* strs[] = {"MIN", "MAX", "DEF"};
	size_t rangeIndex, resolutionIndex;
	const scpi_number_t def = {0.0, SCPI_UNIT_NONE, SCPI_NUM_DEF};

	reset();

	for (rangeIndex = 0; rangeIndex < sizeof(numbers) / sizeof(numbers[0]); ++rangeIndex) {
		reset_counters();
		receivef("CONFIGURE:%s %s", function, strs[rangeIndex]);
		assert_no_scpi_errors();
		asset_no_data();
		check_last_mode(mode, &numbers[rangeIndex], &def);
	}

	for (rangeIndex = 0; rangeIndex < sizeof(numbers) / sizeof(numbers[0]); ++rangeIndex) {
		for (resolutionIndex = 0; resolutionIndex < sizeof(numbers) / sizeof(numbers[0]); ++resolutionIndex) {
			reset_counters();
			receivef("CONFIGURE:%s %s,%s", function, strs[rangeIndex], strs[resolutionIndex]);
			assert_no_scpi_errors();
			asset_no_data();
			check_last_mode(mode, &numbers[rangeIndex], &numbers[resolutionIndex]);
		}
	}
}

static void test_configure_custom_params(const char* function, scpimm_mode_t mode) {
	const scpi_number_t const ranges[] = {
		{0.001, SCPI_UNIT_NONE, SCPI_NUM_NUMBER},
		{0.01, SCPI_UNIT_NONE, SCPI_NUM_NUMBER},
		{0.1, SCPI_UNIT_NONE, SCPI_NUM_NUMBER},
		{1.0, SCPI_UNIT_NONE, SCPI_NUM_NUMBER},
		{10.0, SCPI_UNIT_NONE, SCPI_NUM_NUMBER},
		{100.0, SCPI_UNIT_NONE, SCPI_NUM_NUMBER},
		{1000.0, SCPI_UNIT_NONE, SCPI_NUM_NUMBER}
	};
	const scpi_number_t const resolutions[] = {
		{1.0e-6, SCPI_UNIT_NONE, SCPI_NUM_NUMBER},
		{1.0e-5, SCPI_UNIT_NONE, SCPI_NUM_NUMBER},
		{1.0e-4, SCPI_UNIT_NONE, SCPI_NUM_NUMBER},
		{1.0e-3, SCPI_UNIT_NONE, SCPI_NUM_NUMBER},
		{1.0e-2, SCPI_UNIT_NONE, SCPI_NUM_NUMBER},
		{1.0e-1, SCPI_UNIT_NONE, SCPI_NUM_NUMBER},
		{1.0e-0, SCPI_UNIT_NONE, SCPI_NUM_NUMBER}
	};
	size_t rangeIndex, resolutionIndex;
	const scpi_number_t def = {0.0, SCPI_UNIT_NONE, SCPI_NUM_DEF};

	reset();

	for (rangeIndex = 0; rangeIndex < sizeof(ranges) / sizeof(ranges[0]); ++rangeIndex) {
		reset_counters();
		receivef("CONFIGURE:%s %f", function, ranges[rangeIndex].value);
		assert_no_scpi_errors();
		asset_no_data();
		check_last_mode(mode, &ranges[rangeIndex], &def);
	}

	for (rangeIndex = 0; rangeIndex < sizeof(ranges) / sizeof(ranges[0]); ++rangeIndex) {
		for (resolutionIndex = 0; resolutionIndex < sizeof(resolutions) / sizeof(resolutions[0]); ++resolutionIndex) {
			reset_counters();
			receivef("CONFIGURE:%s %f,%f", function, ranges[rangeIndex].value, resolutions[resolutionIndex].value);
			assert_no_scpi_errors();
			asset_no_data();
			check_last_mode(mode, &ranges[rangeIndex], &resolutions[resolutionIndex]);
		}
	}
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
//		test_configure_fix_params(function, mode);
		//test_configure_custom_params(function, mode);
	}
	if (units) {
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
	test_impl("VOLTAGE:DC", SCPIMM_MODE_DCV, "V");
}

void test_configure_voltage_dc_ratio() {
	test_impl("VOLTAGE:DC:RATIO", SCPIMM_MODE_DCV_RATIO, "V");
}

void test_configure_voltage_ac() {
	test_impl("VOLTAGE:AC", SCPIMM_MODE_ACV, "V");
}

void test_configure_current_dc() {
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

