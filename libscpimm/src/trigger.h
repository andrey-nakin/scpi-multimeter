#ifndef __TRIGGER_H_SCPIMM_
#define	__TRIGGER_H_SCPIMM_

#include <scpi/scpi.h>

scpi_result_t SCPIMM_trigger_source(scpi_t* context);
scpi_result_t SCPIMM_trigger_sourceQ(scpi_t* context);
scpi_result_t SCPIMM_trigger_delay(scpi_t* context);
scpi_result_t SCPIMM_trigger_delayQ(scpi_t* context);
scpi_result_t SCPIMM_trigger_delay_auto(scpi_t* context);
scpi_result_t SCPIMM_trigger_delay_autoQ(scpi_t* context);
scpi_result_t SCPIMM_trigger_count(scpi_t* context);
scpi_result_t SCPIMM_trigger_countQ(scpi_t* context);

#endif	//	__TRIGGER_H_SCPIMM_

