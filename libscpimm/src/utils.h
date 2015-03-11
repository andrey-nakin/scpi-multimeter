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

#define CHECK_SCPI_ERROR(expr) if (SCPI_ERROR_OK != (err = (expr))) return err

void SCPIMM_stop_mesurement(void);
void SCPIMM_clear_return_buffer(void);
size_t SCPIMM_ResultDouble(scpi_t * context, double val);

bool_t expectNoParams(scpi_t* context);

void signalInternalError(scpi_t* context);

bool_t matchPattern(const char * pattern, size_t pattern_len, const char * str, size_t str_len);
bool_t matchCommand(const char * pattern, const char * cmd, size_t len);
size_t strToDouble(const char * str, double * val);

size_t double_to_str(char* dest, double v);

#endif	//	__UTILS_H_SCPIMM_

