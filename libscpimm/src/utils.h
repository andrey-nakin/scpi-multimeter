#ifndef __UTILS_H_SCPIMM_
#define	__UTILS_H_SCPIMM_

#include <stdio.h>
#include <scpi/scpi.h>
#include <scpimm/scpimm.h>

/* 
	Helpful macros to access SCPIMM global context from SCPI command handlers 
	<context> is a scpi_t* value
*/
#define SCPIMM_CONTEXT(context) ((scpimm_context_t*) (context)->user_context)
#define SCPIMM_INTERFACE(context) (SCPIMM_CONTEXT(context)->interface)

void SCPIMM_stop_mesurement(void);
void SCPIMM_clear_return_buffer(void);

/* defined in scpi-parser library */
bool_t matchPattern(const char * pattern, size_t pattern_len, const char * str, size_t str_len);
size_t strToDouble(const char * str, double * val);

#endif	//	__UTILS_H_SCPIMM_

