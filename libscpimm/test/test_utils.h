#ifndef	_TEST_UTILS_SCPIMM_
#define	_TEST_UTILS_SCPIMM_

#include <scpi/scpi.h>
#include "../src/scpimm_internal.h"
#include "default_multimeter.h"

#define FLOAT_DELTA 1.0e-6

#define	CALLED_ONCE	1
#define	NOT_CALLED 0

#define ASSERT_NO_SCPI_ERROR(expr) CU_ASSERT_EQUAL((err = (expr)), SCPI_ERROR_OK); if (err) return
#define ASSERT_EQUAL_BOOL(a, b) CU_ASSERT_TRUE(((a) && (b)) || (!(a) && !(b)))
#define ASSERT_DOUBLE_EQUAL(a, b) CU_ASSERT_DOUBLE_EQUAL(a, b, FLOAT_DELTA)

void init_in_buffer();
void init_test_vars();
void init_scpimm();

void receive(const char* s);
void receivef(const char* fmt, ...);
void dump_in_data();

void clearscpi_errors();

void assert_no_scpi_errors();
void assert_scpi_error(int16_t error);
void assert_in_data(const char* s);
void assert_no_data();
void assert_in_bool(bool_t v);
void assert_in_int(int v);
void assert_in_double(double v);

#define ASSERT_INTERRUPTS_ARE_ENABLED() CU_ASSERT_EQUAL(dm_multimeter_state.interrrupt_disable_counter, 0)

void assert_number_equals(const scpi_number_t* v, const scpi_number_t* expected);

const scpimm_interface_t* scpimm_interface();

#endif	//	_TEST_UTILS_SCPIMM_

