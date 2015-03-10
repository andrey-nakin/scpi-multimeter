#ifndef	_DEFAULT_MULTIMETER_H
#define	_DEFAULT_MULTIMETER_H

/**
 * Dummy multimeter implementation
 */

#include <scpi/scpi.h>
#include <scpimm/scpimm.h>

typedef struct {
	scpimm_mode_t mode;
	bool_t mode_initialized;
	scpimm_mode_params_t mode_params;
	bool_t mode_params_initialized;
} dm_multimeter_state_t;

typedef struct {
	scpimm_mode_t mode;
	scpimm_mode_params_t params;
	bool_t params_is_null;
} dm_set_mode_args_t;

typedef struct {
	scpimm_mode_t mode;
	const double* ranges;
	bool_t ranges_is_null;
	const double* overruns;
	bool_t overruns_is_null;
} dm_get_allowed_ranges_args_t;

typedef struct {
	scpimm_mode_t mode;
	size_t range_index;
	const double* resolutions;
	bool_t resolutions_is_null;
} dm_get_allowed_resolutions_args_t;

typedef struct {
	unsigned set_mode, get_mode;
} dm_counters_t;

extern dm_multimeter_state_t dm_multimeter_state;
extern dm_set_mode_args_t dm_set_mode_last_args;
extern dm_get_allowed_ranges_args_t dm_get_allowed_ranges_last_args;
extern dm_get_allowed_resolutions_args_t dm_get_allowed_resolutions_last_args;
extern scpimm_interface_t dm_interface;
extern dm_counters_t dm_counters;

void dm_init_in_buffer();
char* dm_output_buffer();
void dm_reset_counters();

#endif	//	_DEFAULT_MULTIMETER_H
