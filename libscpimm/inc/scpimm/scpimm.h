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

#define SCPIMM_OVERFLOW 9.90000000E+37

#define	SCPI_ERROR_INTERNAL SCPI_ERROR_PARAMETER_NOT_ALLOWED

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
	bool_t (*set_mode)(scpimm_mode_t mode);

	bool_t (*set_range)(scpimm_mode_t mode, const scpi_number_t* range);

	bool_t (*get_range)(scpimm_mode_t mode, scpi_number_t* range);

	bool_t (*set_resolution)(scpimm_mode_t mode, const scpi_number_t* range);

	bool_t (*get_resolution)(scpimm_mode_t mode, scpi_number_t* range);

	/* 
		Mandatory
		Start measurement
	*/
	void (*trigger)();

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
void SCPIMM_acceptValue(const scpi_number_t* value);
void SCPIMM_parseInBuffer(const char* buf, size_t len);

/* For debug purposes */
scpimm_context_t* SCPIMM_context();
scpi_t* SCPI_context();

#ifdef  __cplusplus
}
#endif

#endif	//	__EXTERNALS_H_SCPIMM


