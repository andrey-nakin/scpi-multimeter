#ifndef __INPUT_H_SCPIMM_
#define	__INPUT_H_SCPIMM_

#include <scpi/scpi.h>

scpi_result_t SCPIMM_do_set_input_impedance_auto(scpi_t* context, bool_t state);

scpi_result_t SCPIMM_input_impedance_auto(scpi_t* context);
scpi_result_t SCPIMM_input_impedance_autoQ(scpi_t* context);

#endif	//	__INPUT_H_SCPIMM_

