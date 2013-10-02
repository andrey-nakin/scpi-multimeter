#include "measure.h"
#include "configure.h"

scpi_result_t SCPIMM_measure_voltage_dcQ(scpi_t* const context) {
	scpi_result_t result = SCPIMM_configure_voltage_dc(context);
	if (SCPI_RES_OK != result) {
		return result;
	}

    SCPI_ResultDouble(context, 1.23);
    
    return SCPI_RES_OK;
}

