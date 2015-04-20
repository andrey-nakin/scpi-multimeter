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

#ifndef __UTILS_H_SCPIMM_
#define	__UTILS_H_SCPIMM_

#include <stdio.h>
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

#define FLOAT_DELTA	1.0e-6
/* 
	Helpful macros to access SCPIMM global context from SCPI command handlers 
	<context> is a scpi_t* value
*/
#define SCPIMM_CONTEXT(context) ((scpimm_context_t*) (context)->user_context)
#define SCPIMM_INTERFACE(context) (SCPIMM_CONTEXT(context)->interface)

#define CHECK_SCPIMM_ERROR(expr) if (SCPIMM_ERROR_OK != (err = (expr))) return err
#define CHECK_AND_PUSH_ERROR(expr) 	if (SCPIMM_ERROR_OK != (err = (expr))) {	\
	    SCPI_ErrorPush(context, err);	\
		return SCPI_RES_ERR;	\
	}

#define	EXPECT_NO_PARAMS(context) if (!expectNoParams(context)) return SCPI_RES_ERR

#define	ATOMIC_READ_BOOL(var) (var)
#define	ATOMIC_WRITE_BOOL(var, value) (var) = (value)
#define	ATOMIC_READ_INT(var) (var)
#define	ATOMIC_WRITE_INT(var, value) (var) = (value)

scpi_bool_t expectNoParams(scpi_t* context);

size_t double_to_str(char* dest, double v);
size_t min_value_index(const double* const values);
size_t max_value_index(const double* const values);
size_t less_or_equal_index(const double* values, const double v);
size_t greater_or_equal_index(const double* values, const double v);

#endif	//	__UTILS_H_SCPIMM_

