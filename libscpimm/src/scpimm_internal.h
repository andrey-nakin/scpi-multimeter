#ifndef	__SCPIMM_INTERNAL_H
#define	__SCPIMM_INTERNAL_H

#include <stdlib.h>
#include <scpimm/scpimm.h>

#define SCPIMM_BUF_LEN 11
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
	bool_t beeper_state;
	bool_t input_impedance_auto_state;
	uint16_t sample_count_num, trigger_count_num, sample_count, trigger_count;
	bool_t infinite_trigger_count;
	scpimm_trig_src_t trigger_src;
	float trigger_delay;
	bool_t trigger_auto_delay;
	scpimm_dst_t dst;
	double buf[SCPIMM_BUF_LEN];
	unsigned buf_count;
	scpimm_state_t state;
	uint32_t state_time;
	bool_t display;
	char display_text[SCPIMM_DISPLAY_LEN + 1];
	uint32_t measurement_timeout;

	scpi_number_t last_measured_value;
	uint32_t measure_start_time;
	int16_t measurement_error;
	bool_t is_first_measured_value;

	struct {
		scpimm_mode_params_t dcv;
		scpimm_mode_params_t dcv_ratio;
		scpimm_mode_params_t acv;
		scpimm_mode_params_t dcc;
		scpimm_mode_params_t acc;
		scpimm_mode_params_t resistance;
		scpimm_mode_params_t fresistance;
	} mode_params;

} scpimm_context_t;

/* For debug purposes */
scpimm_context_t* SCPIMM_context();
scpi_t* SCPI_context();

#endif	//	__SCPIMM_INTERNAL_H
