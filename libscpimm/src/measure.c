#include "measure.h"

scpi_result_t SCPIMM_measure_voltage_dcQ(scpi_t* const context) {
    scpi_number_t param1, param2;
    char bf[15];

    // read first parameter if present
    if (!SCPI_ParamNumber(context, &param1, false)) {
        // do something, if parameter not present
    }

    // read second paraeter if present
    if (!SCPI_ParamNumber(context, &param2, false)) {
        // do something, if parameter not present
    }

    SCPI_ResultDouble(context, 1.23);
    
    return SCPI_RES_OK;
}

