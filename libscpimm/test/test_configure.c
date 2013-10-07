#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>

#define CONTEXT (SCPIMM_context())
#define FLOAT_DELTA 1.0e-6

static int mode_counter;
static uint16_t last_mode;
static float last_range;
static float last_resolution;

static bool_t set_mode(const uint16_t mode, float range, float resolution) {
	++mode_counter;
	last_mode = mode;
	last_range = range;
	last_resolution = resolution;

	return TRUE;
}

static void reset() {
	init_scpimm();
	clearscpi_errors();
	init_test_vars();
	SCPIMM_context()->interface->set_mode = set_mode;

	mode_counter = 0;
	last_mode = 0;
	last_range = -100;
	last_resolution = -100;
}

static float* range_context_var(const uint16_t mode) {
	const uint16_t const modes[] = {SCPIMM_MODE_DCV, SCPIMM_MODE_DCV_RATIO, 
		SCPIMM_MODE_ACV, SCPIMM_MODE_DCC, SCPIMM_MODE_ACC, 
		SCPIMM_MODE_RESISTANCE_2W, SCPIMM_MODE_RESISTANCE_4W, 
		SCPIMM_MODE_FREQUENCY, SCPIMM_MODE_PERIOD};
	scpimm_context_t* const ctx = SCPIMM_context();
	float* const vars[] = {&ctx->dcv_range, &ctx->dcv_ratio_range, &ctx->acv_range, 
		&ctx->dcc_range, &ctx->acc_range, &ctx->resistance_range, 
		&ctx->fresistance_range, &ctx->frequency_range, &ctx->period_range};
	size_t i;
	
	for (i = 0; i < sizeof(modes) / sizeof(modes[0]); ++i) {
		if (modes[i] == mode) {
			return vars[i];
		}
	}

	return NULL;
}

static float* resolution_context_var(const uint16_t mode) {
	const uint16_t const modes[] = {SCPIMM_MODE_DCV, SCPIMM_MODE_DCV_RATIO, 
		SCPIMM_MODE_ACV, SCPIMM_MODE_DCC, SCPIMM_MODE_ACC, 
		SCPIMM_MODE_RESISTANCE_2W, SCPIMM_MODE_RESISTANCE_4W, 
		SCPIMM_MODE_FREQUENCY, SCPIMM_MODE_PERIOD};
	scpimm_context_t* const ctx = SCPIMM_context();
	float* const vars[] = {&ctx->dcv_resolution, &ctx->dcv_ratio_resolution, 
		&ctx->acv_resolution, &ctx->dcc_resolution, &ctx->acc_resolution, 
		&ctx->resistance_resolution, &ctx->fresistance_resolution, 
		&ctx->frequency_resolution, &ctx->period_resolution};
	size_t i;
	
	for (i = 0; i < sizeof(modes) / sizeof(modes[0]); ++i) {
		if (modes[i] == mode) {
			return vars[i];
		}
	}

	return NULL;
}

static void check_last_mode(uint16_t mode, float range, float resolution) {
	const scpimm_context_t* ctx = SCPIMM_context();
	const float* rangeVar = range_context_var(mode);
	const float* resolutionVar = resolution_context_var(mode);

    CU_ASSERT_EQUAL(mode_counter, 1);
    CU_ASSERT_EQUAL(last_mode, mode);
    CU_ASSERT_EQUAL(ctx->mode, mode);
	
	if (range < 0) {
	    CU_ASSERT_EQUAL(last_range, range);
		if (rangeVar) {
			CU_ASSERT_EQUAL(*rangeVar, range);
		}
	} else {
	    CU_ASSERT_DOUBLE_EQUAL(last_range, range, range * FLOAT_DELTA);
		if (rangeVar) {
			CU_ASSERT_DOUBLE_EQUAL(*rangeVar, range, range * FLOAT_DELTA);
		}
	}

	if (resolution < 0) {
		CU_ASSERT_EQUAL(last_resolution, resolution);
		if (resolutionVar) {
			CU_ASSERT_EQUAL(*resolutionVar, resolution);
		}
	} else {
		CU_ASSERT_DOUBLE_EQUAL(last_resolution, resolution, resolution * FLOAT_DELTA);
		if (resolutionVar) {
			CU_ASSERT_DOUBLE_EQUAL(*resolutionVar, resolution, resolution * FLOAT_DELTA);
		}
	}
}

/* configure function with default range and resolution */
static void test_configure_no_params(const char* function, uint16_t mode) {
	float* rangeVar = range_context_var(mode);
	float* resolutionVar = resolution_context_var(mode);

	reset();
	receivef("CONFIGURE:%s\r\n", function);
	assert_no_scpi_errors();
	asset_no_data();
	check_last_mode(mode, SCPIMM_RANGE_DEF, SCPIMM_RESOLUTION_DEF);

	if (rangeVar && resolutionVar) {
		const float customRange = 1.0;
		const float customResolution = 0.1;

		reset();

		*rangeVar = customRange;
		*resolutionVar = customResolution;

		receivef("CONFIGURE:%s\r\n", function);
		assert_no_scpi_errors();
		asset_no_data();
		check_last_mode(mode, customRange, customResolution);
	}
}

/* configure function with default range and resolution */
static void test_configure_fix_params(const char* function, uint16_t mode) {
	const float ranges[] = {SCPIMM_RANGE_MIN, SCPIMM_RANGE_MAX, SCPIMM_RANGE_DEF};
	const float resolutions[] = {SCPIMM_RESOLUTION_MIN, SCPIMM_RESOLUTION_MAX, SCPIMM_RESOLUTION_DEF};
	const char* strs[] = {"MIN", "MAX", "DEF"};
	size_t rangeIndex, resolutionIndex;

	reset();

	for (rangeIndex = 0; rangeIndex < sizeof(ranges) / sizeof(ranges[0]); ++rangeIndex) {
		mode_counter = 0;
		receivef("CONFIGURE:%s %s", function, strs[rangeIndex]);
		assert_no_scpi_errors();
		asset_no_data();
		check_last_mode(mode, ranges[rangeIndex], SCPIMM_RESOLUTION_DEF);
	}

	for (rangeIndex = 0; rangeIndex < sizeof(ranges) / sizeof(ranges[0]); ++rangeIndex) {
		for (resolutionIndex = 0; resolutionIndex < sizeof(resolutions) / sizeof(resolutions[0]); ++resolutionIndex) {
			mode_counter = 0;
			receivef("CONFIGURE:%s %s,%s", function, strs[rangeIndex], strs[resolutionIndex]);
			assert_no_scpi_errors();
			asset_no_data();
			check_last_mode(mode, ranges[rangeIndex], resolutions[resolutionIndex]);
		}
	}
}

static void test_configure_custom_params(const char* function, uint16_t mode) {
	const float ranges[] = {0.001, 0.01, 0.1, 1.0, 10.0, 100.0, 1000.0};
	const float resolutions[] = {1.0e-6, 1.0e-5, 1.0e-4, 1.0e-3, 1.0e-2, 1.0e-1, 1.0e0};
	size_t rangeIndex, resolutionIndex;

	reset();

	for (rangeIndex = 0; rangeIndex < sizeof(ranges) / sizeof(ranges[0]); ++rangeIndex) {
		mode_counter = 0;
		receivef("CONFIGURE:%s %f", function, (double) ranges[rangeIndex]);
		assert_no_scpi_errors();
		asset_no_data();
		check_last_mode(mode, ranges[rangeIndex], SCPIMM_RESOLUTION_DEF);
	}

	for (rangeIndex = 0; rangeIndex < sizeof(ranges) / sizeof(ranges[0]); ++rangeIndex) {
		for (resolutionIndex = 0; resolutionIndex < sizeof(resolutions) / sizeof(resolutions[0]); ++resolutionIndex) {
			mode_counter = 0;
			receivef("CONFIGURE:%s %f,%f", function, (double) ranges[rangeIndex], (double) resolutions[resolutionIndex]);
			assert_no_scpi_errors();
			asset_no_data();
			check_last_mode(mode, ranges[rangeIndex], resolutions[resolutionIndex]);
		}
	}
}

static void test_impl(const char* function, uint16_t mode) {
	test_configure_no_params(function, mode);
	test_configure_fix_params(function, mode);
	test_configure_custom_params(function, mode);
}

int init_suite(void) {
    return 0;
}

int clean_suite(void) {
    return 0;
}

void test_configure_voltage_dc() {
	test_impl("VOLTAGE:DC", SCPIMM_MODE_DCV);
}

void test_configure_voltage_dc_ratio() {
	test_impl("VOLTAGE:DC:RATIO", SCPIMM_MODE_DCV_RATIO);
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
//    if ((NULL == CU_add_test(pSuite, "test configure:voltage:dc", test_configure_voltage_dc))) {
//        CU_cleanup_registry();
//        return CU_get_error();
//    }
    if ((NULL == CU_add_test(pSuite, "test configure:voltage:dc:ratio", test_configure_voltage_dc_ratio))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

