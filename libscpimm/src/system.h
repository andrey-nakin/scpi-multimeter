#ifndef __SYSTEM_H_SCPIMM_
#define	__SYSTEM_H_SCPIMM_

#include <scpi/scpi.h>

scpi_result_t SCPIMM_system_beeper(scpi_t* context);
scpi_result_t SCPIMM_system_beeper_state(scpi_t* context);
scpi_result_t SCPIMM_system_beeper_stateQ(scpi_t* context);
scpi_result_t SCPIMM_system_local(scpi_t* context);
scpi_result_t SCPIMM_system_remote(scpi_t* context);
scpi_result_t SCPIMM_system_rwlock(scpi_t* context);

#endif	//	__SYSTEM_H_SCPIMM_

