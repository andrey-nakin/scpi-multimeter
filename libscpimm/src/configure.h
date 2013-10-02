#ifndef __CONFIGURE_H_SCPIMM_
#define	__CONFIGURE_H_SCPIMM_

#include <scpi/scpi.h>

scpi_result_t SCPIMM_do_configure(scpi_t* context, int mode, float range, float resolution);

scpi_result_t SCPIMM_configureQ(scpi_t* context);
scpi_result_t SCPIMM_configure_voltage_dc(scpi_t* context);
scpi_result_t SCPIMM_configure_voltage_dc_ratio(scpi_t* context);
scpi_result_t SCPIMM_configure_voltage_ac(scpi_t* context);
scpi_result_t SCPIMM_configure_current_dc(scpi_t* context);
scpi_result_t SCPIMM_configure_current_ac(scpi_t* context);
scpi_result_t SCPIMM_configure_resistance(scpi_t* context);
scpi_result_t SCPIMM_configure_fresistance(scpi_t* context);
scpi_result_t SCPIMM_configure_frequency(scpi_t* context);
scpi_result_t SCPIMM_configure_period(scpi_t* context);
scpi_result_t SCPIMM_configure_continuity(scpi_t* context);
scpi_result_t SCPIMM_configure_diode(scpi_t* context);

#endif	//	__CONFIGURE_H_SCPIMM_

