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

#include "route.h"
#include "scpimm_internal.h"
#include "utils.h"

scpi_result_t SCPIMM_route_terminals(scpi_t* const context) {
	int16_t err;

	/* default terminal */
	scpimm_terminal_state_t term = SCPIMM_TERM_FRONT;

	const scpimm_interface_t* const intf = SCPIMM_INTERFACE(context);
	if (intf->get_input_terminal) {
		/* query actual selected terminal */
		CHECK_AND_PUSH_ERROR(intf->get_input_terminal(&term));
	}

	SCPI_ResultString(context, term == SCPIMM_TERM_FRONT ? "FRON" : "REAR");
	return SCPI_RES_OK;
}

