#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "test_utils.h"
#include "../src/utils.h"

static void test_double_to_str_impl(const double value, const char* const expected) {
	char buf[20];

	double_to_str(buf, value);
	CU_ASSERT_STRING_EQUAL(buf, expected);
}

static void test_double_to_str() {
	unsigned f, sign;
	unsigned long m;
	int order;
	char expected[20];
	double v;

	test_double_to_str_impl(+0.0, "+0.000000E+00");
	test_double_to_str_impl(-0.0, "+0.000000E+00");

	for (order = -20; order <= 20; order++) {
		for (sign = 0; sign <= 1; sign++) {
			for (f = 1; f <= 9; ++f) {
				for (m = 0; m <= 999999; m += 4723) {
					sprintf(expected, "%c%d.%06ldE%c%02u",
							sign == 0 ? '+' : '-',
							(int) f,
							(long) m,
							order >= 0 ? '+' : '-',
							order >= 0 ? order : -order);
					sscanf(expected, "%le", &v);
					test_double_to_str_impl(v, expected);
				}
			}
		}
	}

}

int init_suite(void) {
	init_scpimm();
    return 0;
}

int clean_suite(void) {
    return 0;
}

int main() {
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    pSuite = CU_add_suite("GENERIC", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "double_to_str", test_double_to_str))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

