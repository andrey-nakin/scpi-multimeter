/*
    Copyright (c) 2015 Andrey Nakin
    All Rights Reserved

	This file is part of scpi-multimeter library.

	v7-28-arduino is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	v7-28-arduino is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with v7-28-arduino.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef	_TEST_UTILS_SCPIMM_
#define	_TEST_UTILS_SCPIMM_

#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "../src/scpimm_internal.h"
#include "../src/utils.h"
#include "default_multimeter.h"

#define FLOAT_DELTA 1.0e-6

#define	CALLED_ONCE	1
#define	CALLED_TWICE	2
#define	NOT_CALLED 0
#define	NO_USER_DATA	NULL

// test configuration
#define ADD_SUITE(name)	\
    pSuite = CU_add_suite(name, init_suite, clean_suite);	\
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
#define ASSERT_SCPI_ERROR(error) { CU_ASSERT_EQUAL(SCPI_ErrorCount(SCPI_context()), 1); CU_ASSERT_EQUAL(SCPI_ErrorPop(SCPI_context()), error); }
#define ASSERT_NO_RESPONSE() CU_ASSERT_STRING_EQUAL(dm_read_entire_output_buffer(), "");
#define ASSERT_RESPONSE(expected) CU_ASSERT_STRING_EQUAL(dm_read_entire_output_buffer(), expected);
#define ASSERT_BOOL_RESPONSE(expected) CU_ASSERT_STRING_EQUAL(dm_read_entire_output_buffer(), (expected) ? "1\r\n" : "0\r\n");
#define ASSERT_INT_RESPONSE(expected) {double v_; CU_ASSERT_EQUAL(sscanf(dm_read_entire_output_buffer(), "%lg", &v_), 1); CU_ASSERT_EQUAL((int) v_, expected); }
#define ASSERT_DOUBLE_RESPONSE(expected) {double v_; CU_ASSERT_EQUAL(sscanf(dm_read_entire_output_buffer(), "%lg", &v_), 1); CU_ASSERT_DOUBLE_EQUAL(v_, expected, FLOAT_DELTA); }

// required by CU_UNIT
int init_suite(void);
int clean_suite(void);

// send command to test multimeter
void receive(const char* s);
void receivef(const char* fmt, ...);

// clear all SCPI error in FIFO
void clearscpi_errors();

const scpimm_interface_t* scpimm_interface();
void shorten_command_name(char* buf);
void repeat_for_all_dc_modes(void (*test)(const char* func, scpimm_mode_t mode, void* user_data), void* user_data);
void repeat_for_all_modes(void (*test)(const char* func, scpimm_mode_t mode, void* user_data), void* user_data);

#endif	//	_TEST_UTILS_SCPIMM_

