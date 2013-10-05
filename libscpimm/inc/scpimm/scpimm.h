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
  Range constants
******************************************************************************/

#define SCPIMM_RANGE_MIN -1.0
#define SCPIMM_RANGE_MAX -2.0
#define SCPIMM_RANGE_DEF -3.0
#define SCPIMM_RANGE_UNSPECIFIED -4.0

#define SCPIMM_RESOLUTION_MIN SCPIMM_RANGE_MIN
#define SCPIMM_RESOLUTION_MAX SCPIMM_RANGE_MAX
#define SCPIMM_RESOLUTION_DEF SCPIMM_RANGE_DEF
#define SCPIMM_RESOLUTION_UNSPECIFIED SCPIMM_RANGE_UNSPECIFIED

/******************************************************************************
  Types
******************************************************************************/

struct _scpimm_interface_t {
	/*
		Mandatory
		Return bitwise set of modes suported by underlying implementation
	*/
	uint16_t (*supported_modes)(void);

	/* 
		Mandatory
		Set multimeter measurement mode
		<mode> is one of the MM_MODE_XXX constants 
		Return TRUE if mode is set
	*/
	bool_t (*set_mode)(uint16_t mode, float range, float resolution);

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
};

typedef struct _scpimm_interface_t scpimm_interface_t;

struct _scpimm_context_t {
	scpimm_interface_t* interface;
	bool_t beeper_state;
	uint16_t mode;

	float dcv_range;
	float dcv_ratio_range;
	float acv_range;
	float dcc_range;
	float acc_range;
	float resistance_range;
	float fresistance_range;
	float frequency_range;
	float period_range;

	float dcv_resolution;
	float dcv_ratio_resolution;
	float acv_resolution;
	float dcc_resolution;
	float acc_resolution;
	float resistance_resolution;
	float fresistance_resolution;
	float frequency_resolution;
	float period_resolution;
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
void SCPIMM_acceptValue(double value);
void SCPIMM_parseInBuffer(const char* buf, size_t len);

/* For debug purposes */
scpimm_context_t* SCPIMM_context();
scpi_t* SCPI_context();

#ifdef  __cplusplus
}
#endif

#endif	//	__EXTERNALS_H_SCPIMM


