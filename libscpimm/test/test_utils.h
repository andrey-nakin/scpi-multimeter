#ifndef	_TEST_UTILS_SCPIMM_
#define	_TEST_UTILS_SCPIMM_

#include <scpi/scpi.h>

#define FLOAT_DELTA 1.0e-6

void init_in_buffer();
void init_test_vars();
void init_scpimm();

void receive(const char* s);
void receivef(const char* fmt, ...);
void dump_in_data();

void clearscpi_errors();

void assert_no_scpi_errors();
void assert_scpi_error(int16_t error);
void asset_in_data(const char* s);
void asset_no_data();
void asset_in_bool(bool_t v);

void assert_number_equals(const scpi_number_t* v, const scpi_number_t* expected);

#endif	//	_TEST_UTILS_SCPIMM_

