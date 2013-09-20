#include "ieee488.h"

scpi_result_t SCPIMM_IdnQ(scpi_t* const context) {
    SCPI_ResultString(context, "HEWLETT-PACKARD,34401A,0,11-5-2");
    return SCPI_RES_OK;
}

