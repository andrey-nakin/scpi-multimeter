#ifndef __EXTERNALS_H_SCPIMM
#define	__EXTERNALS_H_SCPIMM

#include <stdint.h>

/*
	Process value measured
*/
void MM_acceptValue(double value);
void MM_parseInBuffer(const char* buf, size_t len);

/******************************************************************************
  Multimeter mode constants (to use in MM_setMode)
******************************************************************************/

#define MM_MODE_DCV	0
#define MM_MODE_ACV	1
#define MM_MODE_DCC	0
#define MM_MODE_ACC	1
#define MM_MODE_RESISTANCE_2W	2
#define MM_MODE_RESISTANCE_4W	2

/******************************************************************************
  Following functions must be defined
  in multimeter implementation
******************************************************************************/

/* 
  Set multimeter measurement mode
  <mode> is one of the MM_MODE_XXX constants 
  Return 0 if mode is set
*/
int MM_setMode(const uint8_t mode);

/* 
  Set DCV measurement range
  <mode> is one of the MM_MODE_XXX constants 
*/
void MM_setDCVRange(const float max);

/* 
  Set ACV measurement range
*/
void MM_setACVRange(const float max);

/* 
  Set DCC measurement range
*/
void MM_setDCCRange(const float max);

/* 
  Set ACC measurement range
*/
void MM_setACCRange(const float max);

/* 
  Set resistance measurement range
*/
void MM_setResistanceRange(const float max);

/* 
  Start measurement
*/
void MM_triggerMeasurement();

/* 
  Send response to serial port
*/
size_t MM_send(const uint8_t* buf, size_t len);

#endif	//	__EXTERNALS_H_SCPIMM


