#ifndef __EXTERNALS_H_SCPIMM
#define	__EXTERNALS_H_SCPIMM

#include <stdlib.h>
#include <stdint.h>
#include <scpi/scpi.h>

#ifdef  __cplusplus
extern "C" {
#endif

/******************************************************************************
  Basic constants
******************************************************************************/

#define	SCPI_ERROR_INTERNAL SCPI_ERROR_PARAMETER_NOT_ALLOWED

#define SCPIMM_BUF_LEN 11
//#define SCPIMM_BUF_LEN 513
#define SCPIMM_BUF_CAPACITY (SCPIMM_BUF_LEN - 1)

#define SCPIMM_DISPLAY_LEN 12

/******************************************************************************
  Errot codes
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
#define SCPI_ERROR_NOT_ALLOWED_IN_LOCAL	550

/******************************************************************************
  Multimeter mode constants (to use in MM_setMode)
******************************************************************************/

#define SCPIMM_MODE_UNSPECIFIED	0
#define SCPIMM_MODE_DCV	1
#define SCPIMM_MODE_DCV_RATIO	2
#define SCPIMM_MODE_ACV	4
#define SCPIMM_MODE_DCC	8
#define SCPIMM_MODE_ACC	16
#define SCPIMM_MODE_RESISTANCE_2W	32
#define SCPIMM_MODE_RESISTANCE_4W	64
#define SCPIMM_MODE_FREQUENCY	128
#define SCPIMM_MODE_PERIOD	256
#define SCPIMM_MODE_CONTINUITY	512
#define SCPIMM_MODE_DIODE	1024

/******************************************************************************
  Types
******************************************************************************/

/* Input terminal type */
typedef enum {SCPIMM_TERM_FRONT, SCPIMM_TERM_REAR} scpimm_terminal_state_t;

/* Trigger source type */
typedef enum {SCPIMM_TRIG_BUS, SCPIMM_TRIG_IMM, SCPIMM_TRIG_EXT} scpimm_trig_src_t;

/* Destination of measured values */
typedef enum {SCPIMM_DST_BUF, SCPIMM_DST_OUT} scpimm_dst_t;

/* Destination of measured values */
typedef enum {SCPIMM_STATE_IDLE, SCPIMM_STATE_WAIT_FOR_TRIGGER, SCPIMM_STATE_MEASURE} scpimm_state_t;

/* See SCPIMM_MODE_xxx constants */
typedef uint16_t scpimm_mode_t;

struct _scpimm_interface_t {
	/*
		Mandatory
		Return bitwise set of modes suported by underlying implementation
	*/
	scpimm_mode_t (*supported_modes)(void);

	/* 
		Mandatory
		Set multimeter measurement mode
		<mode> is one of the SCPIMM_MODE_XXX constants 
		Return TRUE if mode is set
	*/
	int16_t (*set_mode)(scpimm_mode_t mode, const scpi_number_t* range, const scpi_number_t* resolution);

	bool_t (*set_range)(scpimm_mode_t mode, const scpi_number_t* range);

	bool_t (*get_range)(scpimm_mode_t mode, scpi_number_t* range);

	bool_t (*set_resolution)(scpimm_mode_t mode, const scpi_number_t* range);

	bool_t (*get_resolution)(scpimm_mode_t mode, scpi_number_t* range);

	/* 
		Mandatory
		Start measurement
	*/
	bool_t (*start_measure)();

	/* 
		Mandatory
		Send response to serial port
	*/
	size_t (*send)(const uint8_t* buf, size_t len);

	/* 
		Optional
		Turn "remote control" mode to on/off
	*/
	void (*remote)(bool_t remote, bool_t lock);

	/* 
		Optional
		Issue a short (up to 500 ms) beep
	*/
	void (*beep)();

	/*
		Optional
		Returns selected input terminal
	*/
	bool_t (*get_input_terminal)(scpimm_terminal_state_t* term);

	/*
		Optional
		Set state of the automatic input impedance selection
	*/
	bool_t (*set_input_impedance_auto)(bool_t state);

};

typedef struct _scpimm_interface_t scpimm_interface_t;

struct _scpimm_context_t {
	scpimm_interface_t* interface;
	bool_t beeper_state;
	scpimm_mode_t mode;
	bool_t input_impedance_auto_state;
	unsigned sample_count_num, trigger_count_num, sample_count, trigger_count;
	bool_t infinite_trigger_count;
	scpimm_trig_src_t trigger_src;
	float trigger_delay;
	bool_t trigger_auto_delay;
	scpimm_dst_t dst;
	double buf[SCPIMM_BUF_LEN];
	unsigned buf_head, buf_tail;
	scpimm_state_t state;
	bool_t display;
	char display_text[SCPIMM_DISPLAY_LEN + 1];

	scpi_number_t dcv_range;
	scpi_number_t dcv_ratio_range;
	scpi_number_t acv_range;
	scpi_number_t dcc_range;
	scpi_number_t acc_range;
	scpi_number_t resistance_range;
	scpi_number_t fresistance_range;
	scpi_number_t frequency_range;
	scpi_number_t period_range;

	scpi_number_t dcv_resolution;
	scpi_number_t dcv_ratio_resolution;
	scpi_number_t acv_resolution;
	scpi_number_t dcc_resolution;
	scpi_number_t acc_resolution;
	scpi_number_t resistance_resolution;
	scpi_number_t fresistance_resolution;
	scpi_number_t frequency_resolution;
	scpi_number_t period_resolution;
};

typedef struct _scpimm_context_t scpimm_context_t;

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
void SCPIMM_parseInBuffer(const char* buf, size_t len);

/* For debug purposes */
scpimm_context_t* SCPIMM_context();
scpi_t* SCPI_context();

#ifdef  __cplusplus
}
#endif

#endif	//	__EXTERNALS_H_SCPIMM


