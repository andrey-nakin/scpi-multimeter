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

#ifndef __ERRORS_H_SCPIMM
#define	__ERRORS_H_SCPIMM

#ifdef	__cplusplus
extern "C" {
#endif

/******************************************************************************
  Error codes
******************************************************************************/

//#define SCPIMM_ERROR_OK	0

#define SCPIMM_LIST_OF_ERRORS \
    X(SCPIMM_ERROR_SYNTAX,               -102, "Syntax error")                   \
    X(SCPIMM_ERROR_INVALID_SEPARATOR,    -103, "Invalid separator")              \
    X(SCPIMM_ERROR_UNDEFINED_HEADER,     -113, "Undefined header")               \
    X(SCPIMM_ERROR_PARAMETER_NOT_ALLOWED,-108, "Parameter not allowed")          \
    X(SCPIMM_ERROR_MISSING_PARAMETER,    -109, "Missing parameter")              \
    X(SCPIMM_ERROR_INVALID_SUFFIX,       -131, "Invalid suffix")                 \
    X(SCPIMM_ERROR_SUFFIX_NOT_ALLOWED,   -138, "Suffix not allowed")             \
    X(SCPIMM_ERROR_EXECUTION_ERROR,      -200, "Execution error")                \
    X(SCPIMM_ERROR_TRIGGER_IGNORED,		 -211, "Trigger ignored")				 \
    X(SCPIMM_ERROR_INIT_IGNORED,	 	 -213, "Init ignored")					 \
    X(SCPIMM_ERROR_TRIGGER_DEADLOCK,	 -214, "Trigger deadlock")				 \
    X(SCPIMM_ERROR_DATA_OUT_OF_RANGE, 	 -222, "Data out of range")				 \
    X(SCPIMM_ERROR_TOO_MUCH_DATA,	  	 -223, "Too much data")					 \
    X(SCPIMM_ERROR_ILLEGAL_PARAMETER_VALUE,-224,"Illegal parameter value")		 \
    X(SCPIMM_ERROR_DATA_STALE,		 	 -230, "Data stale")					 \
    X(SCPIMM_ERROR_INSUFFICIENT_MEMORY,	  531, "Insufficient memory")			 \
    X(SCPIMM_ERROR_CANNOT_ACHIEVE_REQUESTED_RESOLUTION, 532, "Cannot achieve requested resolution") \
    X(SCPIMM_ERROR_NOT_ALLOWED_IN_LOCAL,  550, "Not allowed in local")			 \
    X(SCPIMM_ERROR_IO_PROCESSOR_DOES_NOT_RESPOND, 625, "IO processor does not respond")	\
    X(SCPIMM_ERROR_INTERNAL, 			  555, "Internal error")	\

typedef enum {
#define X(def, val, str) def = val,
SCPIMM_LIST_OF_ERRORS
SCPIMM_ERROR_OK = 0
#undef X
} scpimm_error_t;

#ifdef	__cplusplus
}
#endif

#endif	//	__ERRORS_H_SCPIMM
