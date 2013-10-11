#ifndef __DMM_H_SCPIMM_
#define	__DMM_H_SCPIMM_

#include <scpi/scpi.h>

scpi_result_t SCPIMM_measure_preset(scpi_t* context);

scpi_result_t SCPIMM_initiate(scpi_t* context);
scpi_result_t SCPIMM_readQ(scpi_t* context);
scpi_result_t SCPIMM_fetchQ(scpi_t* context);

#endif	//	__DMM_H_SCPIMM_

