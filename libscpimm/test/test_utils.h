#ifndef	_TEST_UTILS_SCPIMM_
#define	_TEST_UTILS_SCPIMM_

#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "../src/scpimm_internal.h"
#include "default_multimeter.h"

#define FLOAT_DELTA 1.0e-6

#define	CALLED_ONCE	1
#define	NOT_CALLED 0

// test configuration
#define ADD_SUITE(name)	\
    pSuite = CU_add_suite("CONFIGURE", init_suite, clean_suite);	\
    if (NULL == pSuite) {	\
        CU_cleanup_registry();	\
        return CU_get_error();	\
    }

#define ADD_TEST(name)	\
    if ((NULL == CU_add_test(pSuite, #name, name))) {	\
        CU_cleanup_registry();	\
        return CU_get_error();	\
    }

#define RUN_ALL_TESTS()	\
	CU_basic_set_mode(CU_BRM_VERBOSE),	\
    CU_basic_run_tests(),	\
    CU_cleanup_registry(),	\
    CU_get_error()

// assertion utilities

#define CHECK_NO_SCPI_ERROR(expr) CU_ASSERT_EQUAL((err = (expr)), SCPI_ERROR_OK); if (err) return

#define ASSERT_EQUAL_BOOL(a, b) CU_ASSERT_TRUE(((a) && (b)) || (!(a) && !(b)))
#define ASSERT_DOUBLE_EQUAL(a, b) CU_ASSERT_DOUBLE_EQUAL(a, b, FLOAT_DELTA)
#define ASSERT_INTERRUPTS_ARE_ENABLED() CU_ASSERT_EQUAL(dm_multimeter_state.interrrupt_disable_counter, 0)
#define ASSERT_NO_SCPI_ERRORS() CU_ASSERT_EQUAL(SCPI_ErrorCount(SCPI_context()), 0)
#define ASSERT_SCPI_ERROR(error)	\
	CU_ASSERT_EQUAL(SCPI_ErrorCount(SCPI_context()), 1);	\
	CU_ASSERT_EQUAL(SCPI_ErrorPop(SCPI_context()), error)
#define ASSERT_NO_RESPONSE() CU_ASSERT_STRING_EQUAL(dm_output_buffer(), "");
#define ASSERT_RESPONSE(expected) CU_ASSERT_STRING_EQUAL(dm_output_buffer(), expected);
#define ASSERT_BOOL_RESPONSE(expected) CU_ASSERT_STRING_EQUAL(dm_output_buffer(), (expected) ? "1\r\n" : "0\r\n");

//void assert_in_data(const char* s);
//void assert_in_bool(bool_t v);
//void assert_in_int(int v);
//void assert_in_double(double v);
//void assert_number_equals(const scpi_number_t* v, const scpi_number_t* expected);

// required by CU_UNIT
int init_suite(void);
int clean_suite(void);

// send command to test multimeter
void receive(const char* s);
void receivef(const char* fmt, ...);

// clear all SCPI error in FIFO
void clearscpi_errors();

const scpimm_interface_t* scpimm_interface();

#endif	//	_TEST_UTILS_SCPIMM_

