#ifndef	_DEFAULT_MULTIMETER_H
#define	_DEFAULT_MULTIMETER_H

/**
 * Dummy multimeter implementation
 */

#include <scpi/scpi.h>
#include <scpimm/scpimm.h>
#include "../src/scpimm_internal.h"

typedef enum {
	DM_MEASUREMENT_TYPE_ASYNC, DM_MEASUREMENT_TYPE_SYNC
} dm_measurement_type_t;

typedef struct {
	scpimm_mode_t mode;
	scpi_bool_t mode_initialized;
	scpimm_mode_params_t mode_params;
	scpi_bool_t mode_params_initialized;
	unsigned interrrupt_disable_counter;
	unsigned measurement_failure_counter;
} dm_multimeter_state_t;

typedef struct {
	scpimm_mode_t mode;
	scpimm_mode_params_t params;
	scpi_bool_t params_is_null;
} dm_set_mode_args_t;

typedef struct {
	scpimm_mode_t mode;
	const double* ranges;
	scpi_bool_t ranges_is_null;
	const double* overruns;
	scpi_bool_t overruns_is_null;
} dm_get_allowed_ranges_args_t;

typedef struct {
	scpimm_mode_t mode;
	size_t range_index;
	const double* resolutions;
	scpi_bool_t resolutions_is_null;
} dm_get_allowed_resolutions_args_t;

typedef struct {
	scpi_bool_t remote;
	scpi_bool_t lock;
} dm_remote_args_t;

typedef struct {
	const char* txt;
} dm_display_text_args_t;

typedef struct {
	unsigned set_mode, get_mode, get_allowed_ranges, get_allowed_resolutions,
		start_measure, set_interrupt_status, remote, beep, display_text, setup, reset;
} dm_counters_t;

typedef double (*dm_measurement_func_t)(uint32_t time);

typedef struct {
	dm_measurement_type_t measurement_type;
	dm_measurement_func_t measurement_func;
	uint32_t measurement_duration;
} dm_multimeter_config_t;

extern dm_multimeter_state_t dm_multimeter_state;
extern char dm_display[SCPIMM_DISPLAY_LEN + 1];

extern dm_set_mode_args_t dm_set_mode_last_args;
extern dm_get_allowed_ranges_args_t dm_get_allowed_ranges_last_args;
extern dm_get_allowed_resolutions_args_t dm_get_allowed_resolutions_last_args;
extern dm_remote_args_t dm_remote_args;
extern dm_display_text_args_t dm_display_text_args;

extern scpimm_interface_t dm_interface;
extern dm_counters_t dm_counters;
extern dm_multimeter_config_t dm_multimeter_config;

const char* dm_read_entire_output_buffer();
void dm_reset_counters();
void dm_reset_args();

double dm_measurement_func_const(uint32_t time);

#endif	//	_DEFAULT_MULTIMETER_H
