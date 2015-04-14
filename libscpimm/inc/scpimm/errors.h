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

/******************************************************************************
  Error codes
******************************************************************************/

#define SCPI_ERROR_OK	0

#define SCPI_ERROR_TRIGGER_IGNORED	-211
#define SCPI_ERROR_INIT_IGNORED	-213
#define SCPI_ERROR_TRIGGER_DEADLOCK	-214
#define SCPI_ERROR_DATA_OUT_OF_RANGE	-222
#define SCPI_ERROR_TOO_MUCH_DATA	-223
#define SCPI_ERROR_ILLEGAL_PARAMETER_VALUE	-224
#define SCPI_ERROR_DATA_STALE	-230
#define SCPI_ERROR_INSUFFICIENT_MEMORY	531
#define SCPI_ERROR_CANNOT_ACHIEVE_REQUESTED_RESOLUTION	532
#define SCPI_ERROR_NOT_ALLOWED_IN_LOCAL	550
#define SCPI_ERROR_IO_PROCESSOR_DOES_NOT_RESPOND 625

#define SCPI_ERROR_UNKNOWN	551
#define SCPI_ERROR_INTERNAL_START	555

#endif	//	__ERRORS_H_SCPIMM
