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

int test_generic() {
    CU_pSuite pSuite = NULL;

    ADD_SUITE("GENERIC");

    /* Add the tests to the suite */
    ADD_TEST(test_double_to_str);

    return 0;
}

#ifdef	NO_GLOBAL_TEST

int main() {

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    if (test_generic()) {
    	return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    return RUN_ALL_TESTS();
}

#endif
