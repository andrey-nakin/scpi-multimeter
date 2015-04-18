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

#ifndef	__SCPIMM_INTERNAL_H
#define	__SCPIMM_INTERNAL_H

#include <stdlib.h>
#include <scpimm/scpimm.h>

#define SCPIMM_BUF_LEN 512
#define SCPIMM_BUF_CAPACITY SCPIMM_BUF_LEN

#define SCPIMM_DISPLAY_LEN 12

#define	MAX_SAMPLE_COUNT	50000
#define	MAX_TRIGGER_COUNT	MAX_SAMPLE_COUNT

/******************************************************************************
  Types
******************************************************************************/

/* Trigger source type */
typedef enum {SCPIMM_TRIG_BUS, SCPIMM_TRIG_IMM, SCPIMM_TRIG_EXT} scpimm_trig_src_t;

/* Destination of measured values */
typedef enum {SCPIMM_DST_BUF, SCPIMM_DST_OUT} scpimm_dst_t;

/* Destination of measured values */
typedef enum {SCPIMM_STATE_IDLE, SCPIMM_STATE_WAIT_FOR_TRIGGER, SCPIMM_STATE_TRIGGER_DELAY, SCPIMM_STATE_MEASURING, SCPIMM_STATE_MEASURED} scpimm_state_t;

typedef struct {
	scpimm_interface_t* interface;
	scpi_bool_t beeper_state;
	uint16_t sample_count_num, trigger_count_num, sample_count, trigger_count;
	scpi_bool_t infinite_trigger_count;
	scpimm_trig_src_t trigger_src;
	float trigger_delay;
	scpi_bool_t trigger_auto_delay;
	scpimm_dst_t dst;
	double buf[SCPIMM_BUF_LEN];
	unsigned buf_count;
	scpimm_state_t state;
	uint32_t state_time;
	scpi_bool_t display;
	char display_text[SCPIMM_DISPLAY_LEN + 1];
	uint32_t measurement_timeout;

	scpi_number_t last_measured_value;
	int16_t measurement_error;

} scpimm_context_t;

/* For debug purposes */
scpimm_context_t* SCPIMM_context();
scpi_t* SCPI_context();

#endif	//	__SCPIMM_INTERNAL_H
