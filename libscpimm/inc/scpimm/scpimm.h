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

#ifndef __EXTERNALS_H_SCPIMM
#define	__EXTERNALS_H_SCPIMM

#include <stdint.h>
#include <scpi/scpi.h>
#include "scpimm/errors.h"

#ifdef  __cplusplus
extern "C" {
#endif

/******************************************************************************
  Types
******************************************************************************/

/* Input terminal type */
typedef enum {SCPIMM_TERM_FRONT, SCPIMM_TERM_REAR} scpimm_terminal_state_t;

/* Multimeter mode type */
typedef enum {SCPIMM_MODE_DCV, SCPIMM_MODE_DCV_RATIO, SCPIMM_MODE_ACV, SCPIMM_MODE_ACV_RATIO, SCPIMM_MODE_DCC, SCPIMM_MODE_ACC,
	SCPIMM_MODE_RESISTANCE_2W, SCPIMM_MODE_RESISTANCE_4W} scpimm_mode_t;

typedef enum {SCPIMM_PARAM_RANGE, SCPIMM_PARAM_RANGE_OVERRUN, SCPIMM_PARAM_RESOLUTION, SCPIMM_PARAM_NPLC} scpimm_numeric_param_t;

typedef enum {SCPIMM_PARAM_RANGE_AUTO, SCPIMM_PARAM_ZERO_AUTO, SCPIMM_PARAM_ZERO_AUTO_ONCE, SCPIMM_PARAM_INPUT_IMPEDANCE_AUTO, SCPIMM_PARAM_REMOTE, SCPIMM_PARAM_LOCK} scpimm_bool_param_t;

typedef struct _scpimm_mode_params_t {
	size_t range_index;
	scpi_bool_t auto_range;
	size_t resolution_index;
} scpimm_mode_params_t;

typedef struct {

	/*
		Mandatory
		Initializes multimeter. Call once when multimeter is started
	*/
	int16_t (*setup)();

	/* 
		Mandatory
		Set multimeter measurement mode
		<mode> is one of the SCPIMM_MODE_XXX constants 
		Return TRUE if mode is set
	*/
	int16_t (*set_mode)(scpimm_mode_t mode, const scpimm_mode_params_t* params);

	/**
		Mandatory
	 * Query current mode and its parameters
	 * mode - variable to put mode into. If NULL, mode is not required by calling side.
	 */
	int16_t (*get_mode)(scpimm_mode_t* mode);

	int16_t (*get_allowed_resolutions)(scpimm_mode_t mode, size_t range_index, const double** resolutions);

	/* 
		Mandatory
		Start measurement
	*/
	int16_t (*start_measure)();

	/* 
		Mandatory
		Send response to serial port
	*/
	size_t (*send)(const uint8_t* buf, size_t len);

	/* 
		Mandatory
		Returns relative current time in milliseconds
	*/
	int16_t (*get_milliseconds)(uint32_t* tm);

	/*
		Mandatory
		Disables or enables interrupts
	*/
	int16_t (*set_interrupt_status)(scpi_bool_t disabled);

	/*
		Mandatory
		Get boolean value of global measurement parameter
	*/
	int16_t (*get_global_bool_param)(scpimm_bool_param_t param, scpi_bool_t* value);

	/*
		Mandatory
		Set measurement parameter globally, i.e. for all modes
	*/
	int16_t (*set_global_bool_param)(scpimm_bool_param_t param, scpi_bool_t value);

	/*
		Mandatory
	*/
	int16_t (*get_bool_param)(scpimm_mode_t mode, scpimm_bool_param_t param, scpi_bool_t* value);

	/*
		Mandatory
	*/
	int16_t (*set_bool_param)(scpimm_mode_t mode, scpimm_bool_param_t param, scpi_bool_t value);

	/*
		Mandatory
	*/
	int16_t (*get_numeric_param_values)(scpimm_mode_t mode, scpimm_numeric_param_t param, const double** values);

	/*
		Mandatory
	*/
	int16_t (*get_numeric_param)(scpimm_mode_t mode, scpimm_numeric_param_t param, size_t* value_index);

	/*
		Mandatory
	*/
	int16_t (*set_numeric_param)(scpimm_mode_t mode, scpimm_numeric_param_t param, size_t value_index);

	/*
		Optional
		Resets multimeter. Call when multimeter is started or *RST command is invoked
	*/
	int16_t (*reset)();

	/* 
		Optional
		Issue a short (up to 500 ms) beep
	*/
	int16_t (*beep)();

	/*
		Optional
		Returns selected input terminal
	*/
	int16_t (*get_input_terminal)(scpimm_terminal_state_t* term);

	/*
		Optional
		Displays a text on multimeter's display
	*/
	int16_t (*display_text)(const char* txt);

	/*
		Optional
		Returns multimeter identification string, e.g. HEWLETT-PACKARD,34401A,0,11-5-2
	*/
	const char* (*get_idn)();

	/*
		Optional
		Performs a multimeter's internal test
	*/
	int16_t (*test)();

} scpimm_interface_t;

/******************************************************************************
  Public functions
******************************************************************************/
/*
	Initialize multimeter
*/
void SCPIMM_setup(const scpimm_interface_t*);

/*
	Process value measured
*/
void SCPIMM_read_value(const scpi_number_t* value);
void SCPIMM_parse_in_buffer(const char* buf, size_t len);
void SCPIMM_external_trigger();
void SCPIMM_yield();

#ifdef  __cplusplus
}
#endif

#endif	//	__EXTERNALS_H_SCPIMM


