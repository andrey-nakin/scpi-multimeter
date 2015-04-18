/*
    Copyright (c) 2015 Andrey Nakin
    All Rights Reserved

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

#include "ieee488.h"
#include "scpimm_internal.h"
#include "dmm.h"
#include "utils.h"

scpi_result_t SCPIMM_IdnQ(scpi_t* const context) {
	const scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
	const scpimm_interface_t* const intf = ctx->interface;

    SCPI_ResultString(context, intf->get_idn ? intf->get_idn() : "HEWLETT-PACKARD,34401A,0,11-5-2");
    return SCPI_RES_OK;
}

scpi_result_t SCPIMM_opcQ(scpi_t* const context) {
	int16_t err;
	CHECK_AND_PUSH_ERROR(SCPIMM_wait_for_idle(SCPIMM_CONTEXT(context)));
    SCPI_ResultInt(context, 1);
    return SCPI_RES_OK;
}

scpi_result_t SCPIMM_tstQ(scpi_t* const context) {
	scpimm_interface_t* const intf = SCPIMM_INTERFACE(context);
	int16_t err = SCPIMM_ERROR_OK;

	if (intf->test) {
		if (SCPIMM_ERROR_OK != (err = intf->test())) {
			SCPI_ErrorPush(context, err);
		}
	}

    SCPI_ResultInt(context, SCPIMM_ERROR_OK == err ? 0 : 1);

    return SCPIMM_ERROR_OK == err ? SCPI_RES_OK : SCPI_RES_ERR;
}
