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

#ifndef __SYSTEM_H_SCPIMM_
#define	__SYSTEM_H_SCPIMM_

#include <scpi/scpi.h>

scpi_result_t SCPIMM_system_beeper(scpi_t* context);
scpi_result_t SCPIMM_system_beeper_state(scpi_t* context);
scpi_result_t SCPIMM_system_beeper_stateQ(scpi_t* context);
scpi_result_t SCPIMM_system_local(scpi_t* context);
scpi_result_t SCPIMM_system_remote(scpi_t* context);
scpi_result_t SCPIMM_system_rwlock(scpi_t* context);

int16_t SCPIMM_set_remote(scpi_t* context, scpi_bool_t remote, scpi_bool_t lock);

#endif	//	__SYSTEM_H_SCPIMM_

