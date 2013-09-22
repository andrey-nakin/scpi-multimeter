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

#endif	//	__UTILS_H_SCPIMM_

