#ifndef	_TEST_UTILS_SCPIMM_
#define	_TEST_UTILS_SCPIMM_

#include <scpi/scpi.h>

void init_in_buffer();
void init_test_vars();
void init_scpimm();

void receive(const char* s);
void dump_in_data();

void clearscpi_errors();

void assert_no_scpi_errors();
void asset_in_data(const char* s);
void asset_no_data();
void asset_in_bool(bool_t v);

#endif	//	_TEST_UTILS_SCPIMM_

