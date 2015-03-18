#ifndef __MEASURE_H_SCPIMM_
#define	__MEASURE_H_SCPIMM_

#include <scpi/scpi.h>

scpi_result_t SCPIMM_measure_voltage_dcQ(scpi_t* context);
scpi_result_t SCPIMM_measure_voltage_dc_ratioQ(scpi_t* context);
scpi_result_t SCPIMM_measure_voltage_acQ(scpi_t* context);
scpi_result_t SCPIMM_measure_current_dcQ(scpi_t* context);
scpi_result_t SCPIMM_measure_current_acQ(scpi_t* context);
scpi_result_t SCPIMM_measure_resistanceQ(scpi_t* context);
scpi_result_t SCPIMM_measure_fresistanceQ(scpi_t* context);

#endif	//	__MEASURE_H_SCPIMM_

