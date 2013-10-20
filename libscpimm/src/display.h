#ifndef __DISPLAY_H_SCPIMM_
#define	__DISPLAY_H_SCPIMM_

#include <scpi/scpi.h>

scpi_result_t SCPIMM_display(scpi_t* context);
scpi_result_t SCPIMM_displayQ(scpi_t* context);
scpi_result_t SCPIMM_display_text(scpi_t* context);
scpi_result_t SCPIMM_display_textQ(scpi_t* context);
scpi_result_t SCPIMM_display_text_clear(scpi_t* context);

#endif	//	__DISPLAY_H_SCPIMM_

