#ifndef __EXTERNALS_H_SCPIMM
#define	__EXTERNALS_H_SCPIMM

#include <stdint.h>
#include <scpi/scpi.h>
#include "errors.h"

#ifdef  __cplusplus
extern "C" {
#endif

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

/* See SCPIMM_MODE_xxx constants */
typedef uint16_t scpimm_mode_t;

typedef struct _scpimm_mode_params_t {
	size_t range_index;
	bool_t auto_range;
	size_t resolution_index;
} scpimm_mode_params_t;

typedef struct {

	/*
		Mandatory
		Initializes multimeter. Call when multimeter is started or *RST command is invoked
	*/
	int16_t (*reset)();

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
	 * dest - variable to put mode parameters into. If NULL, parameters are not required by calling side.
	 */
	int16_t (*get_mode)(scpimm_mode_t* mode, scpimm_mode_params_t* dest);

	int16_t (*get_allowed_ranges)(scpimm_mode_t mode, const double** ranges, const double** overruns);
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
		Sleeps for a given period in ms
	*/
	int16_t (*sleep_milliseconds)(uint32_t ms);

	/*
		Mandatory
		Disables or enables interrupts
	*/
	int16_t (*set_interrupt_status)(bool_t disabled);

	/*
		Optional
		Turn "remote control" mode to on/off
	*/
	int16_t (*remote)(bool_t remote, bool_t lock);

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


