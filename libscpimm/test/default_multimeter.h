#ifndef	_DEFAULT_MULTIMETER_H
#define	_DEFAULT_MULTIMETER_H

/**
 * Dummy multimeter implementation
 */

#include <scpi/scpi.h>
#include <scpimm/scpimm.h>

typedef struct {
	scpimm_mode_t mode;
	scpimm_mode_params_t mode_params;
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

int16_t dm_set_mode(scpimm_mode_t mode, const scpimm_mode_params_t* const params) {
int16_t dm_get_possible_range(scpimm_mode_t mode, double* min_range, double* max_range);
int16_t dm_get_possible_resolution(scpimm_mode_t mode, double range, double* min_resolution, double* max_resolution);

extern dm_multimeter_state_t dm_multimeter_state;
extern dm_set_mode_args_t dm_set_mode_last_args;
extern dm_get_possible_range_args_t dm_get_possible_range_last_args;
extern dm_get_possible_resolution_args_t dm_get_possible_resolution_last_args;

#endif	//	_DEFAULT_MULTIMETER_H
