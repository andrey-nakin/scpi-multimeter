#ifndef __EXTERNALS_H_SCPIMM
#define	__EXTERNALS_H_SCPIMM

#include <stdint.h>

/*
	Initialize multimeter
*/
void SCPIMM_setup();

/*
	Process value measured
*/
void SCPIMM_acceptValue(double value);
void SCPIMM_parseInBuffer(const char* buf, size_t len);

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
  Following functions must be defined
  in multimeter implementation
******************************************************************************/

/* 
  Set multimeter measurement mode
  <mode> is one of the MM_MODE_XXX constants 
  Return 0 if mode is set
*/
int SCPIMM_setMode(const uint8_t mode);

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

/* 
  Start measurement
*/
void SCPIMM_triggerMeasurement();

/* 
  Send response to serial port
*/
size_t SCPIMM_send(const uint8_t* buf, size_t len);

/* 
  Turn "remote control" mode to on/off
*/
void SCPIMM_remote(const bool remote);

/* 
  Issue a short (up to 500 ms) beep
*/
void SCPIMM_beep();

#endif	//	__EXTERNALS_H_SCPIMM


