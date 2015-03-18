#ifndef __CONFIGURE_H_SCPIMM_
#define	__CONFIGURE_H_SCPIMM_

#include <scpi/scpi.h>
#include <scpimm/scpimm.h>

scpi_result_t SCPIMM_do_configure(scpi_t* context, scpimm_mode_t mode, const scpi_number_t* range, const scpi_number_t* resolution);

scpi_result_t SCPIMM_configureQ(scpi_t* context);
scpi_result_t SCPIMM_configure_voltage_dc(scpi_t* context);
scpi_result_t SCPIMM_configure_voltage_dc_ratio(scpi_t* context);
scpi_result_t SCPIMM_configure_voltage_ac(scpi_t* context);
scpi_result_t SCPIMM_configure_current_dc(scpi_t* context);
scpi_result_t SCPIMM_configure_current_ac(scpi_t* context);
scpi_result_t SCPIMM_configure_resistance(scpi_t* context);
scpi_result_t SCPIMM_configure_fresistance(scpi_t* context);

#endif	//	__CONFIGURE_H_SCPIMM_

