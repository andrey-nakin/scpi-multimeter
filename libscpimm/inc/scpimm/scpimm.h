#ifndef __EXTERNALS_H_SCPIMM
#define	__EXTERNALS_H_SCPIMM

#include <stdlib.h>
#include <stdint.h>
#include <scpi/scpi.h>

/******************************************************************************
  Basic constants
******************************************************************************/

#define SCPIMM_OVERFLOW 9.90000000E+37

/******************************************************************************
  Multimeter mode constants (to use in MM_setMode)
******************************************************************************/

#define SCPIMM_MODE_DCV	0
#define SCPIMM_MODE_DCV_RATIO	1
#define SCPIMM_MODE_ACV	2
#define SCPIMM_MODE_DCC	3
#define SCPIMM_MODE_ACC	4
#define SCPIMM_MODE_RESISTANCE_2W	5
#define SCPIMM_MODE_RESISTANCE_4W	6

/******************************************************************************
  Range constants
******************************************************************************/

#define SCPIMM_RANGE_MIN -1.0
#define SCPIMM_RANGE_MAX -2.0
#define SCPIMM_RANGE_DEF -3.0

/******************************************************************************
  Types
******************************************************************************/

struct _scpimm_interface_t {
	/* 
		Mandatory
		Set multimeter measurement mode
		<mode> is one of the MM_MODE_XXX constants 
		Return 0 if mode is set
	*/
	int (*set_mode)(uint8_t mode);

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
	const scpimm_interface_t* interface;
	bool_t beeper_state;
	float dcv_range;
	float dcv_ratio_range;
	float acv_range;
	float resistance_range;
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

/******************************************************************************
  Following functions must be defined
  in multimeter implementation
******************************************************************************/

/* 
  Set DCV measurement range
  <mode> is one of the MM_MODE_XXX constants 
*/
void SCPIMM_setDCVRange(const float max);

/* 
  Set ACV measurement range
*/
void SCPIMM_setACVRange(const float max);

/* 
  Set DCC measurement range
*/
void SCPIMM_setDCCRange(const float max);

/* 
  Set ACC measurement range
*/
void SCPIMM_setACCRange(const float max);

/* 
  Set resistance measurement range
*/
void SCPIMM_setResistanceRange(const float max);

#endif	//	__EXTERNALS_H_SCPIMM


