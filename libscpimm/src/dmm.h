#ifndef __DMM_H_SCPIMM_
#define	__DMM_H_SCPIMM_

#include <scpi/scpi.h>
#include "scpimm_internal.h"

scpi_result_t SCPIMM_measure_preset(scpi_t* context);
scpi_result_t SCPIMM_set_state(scpi_t* context, scpimm_state_t new_state);
scpimm_state_t SCPIMM_get_state(scpi_t* context);
void SCPIMM_read_value(const scpi_number_t* value);

scpi_result_t SCPIMM_initiate(scpi_t* context);
scpi_result_t SCPIMM_readQ(scpi_t* context);
scpi_result_t SCPIMM_fetchQ(scpi_t* context);
scpi_result_t SCPIMM_trg(scpi_t* context);
scpi_result_t SCPIMM_data_pointsQ(scpi_t* context);

int16_t SCPIMM_set_mode(scpi_t* context, scpimm_mode_t mode, const scpi_number_t* range, scpi_bool_t auto_detect_auto_range, const scpi_bool_t* auto_range, const scpi_number_t* resolution);
const char* SCPIMM_mode_name(scpimm_mode_t mode);

#endif	//	__DMM_H_SCPIMM_

