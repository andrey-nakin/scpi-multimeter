/*
	This file is part of scpi-multimeter library.

	v7-28-arduino is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	v7-28-arduino is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with v7-28-arduino.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TRIGGER_H_SCPIMM_
#define	__TRIGGER_H_SCPIMM_

#include <scpi/scpi.h>

scpi_result_t SCPIMM_trigger_source(scpi_t* context);
scpi_result_t SCPIMM_trigger_sourceQ(scpi_t* context);
scpi_result_t SCPIMM_trigger_delay(scpi_t* context);
scpi_result_t SCPIMM_trigger_delayQ(scpi_t* context);
scpi_result_t SCPIMM_trigger_delay_auto(scpi_t* context);
scpi_result_t SCPIMM_trigger_delay_autoQ(scpi_t* context);
scpi_result_t SCPIMM_trigger_count(scpi_t* context);
scpi_result_t SCPIMM_trigger_countQ(scpi_t* context);

#endif	//	__TRIGGER_H_SCPIMM_

