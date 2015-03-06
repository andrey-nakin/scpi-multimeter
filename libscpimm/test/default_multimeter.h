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
	double min_range;
	bool_t min_range_is_null;
	double max_range;
	bool_t max_range_is_null;
} dm_get_possible_range_args_t;

typedef struct {
	scpimm_mode_t mode;
	double range;
	double min_resolution;
	bool_t min_resolution_is_null;
	double max_resolution;
	bool_t max_resolution_is_null;
} dm_get_possible_resolution_args_t;

typedef struct {
	unsigned set_mode, get_mode;
} dm_counters_t;

extern dm_multimeter_state_t dm_multimeter_state;
extern dm_set_mode_args_t dm_set_mode_last_args;
extern dm_get_possible_range_args_t dm_get_possible_range_last_args;
extern dm_get_possible_resolution_args_t dm_get_possible_resolution_last_args;
extern scpimm_interface_t dm_interface;
extern dm_counters_t dm_counters;

void dm_init_in_buffer();
char* dm_output_buffer();
void dm_reset_counters();

#endif	//	_DEFAULT_MULTIMETER_H
