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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <scpi/config.h>
#include "scpimm_internal.h"
#include "utils.h"

size_t strToDouble(const char * str, double * val);
size_t patternSeparatorShortPos(const char * pattern, size_t len);
scpi_bool_t compareStr(const char * str1, size_t len1, const char * str2, size_t len2);
scpi_bool_t matchPattern(const char * pattern, size_t pattern_len, const char * str, size_t str_len);

static size_t writeData(scpi_t * context, const char * data, size_t len) {
    return context->interface->write(context, data, len);
}

static size_t writeDelimiter(scpi_t * context) {
    if (context->output_count > 0) {
        return writeData(context, ", ", 2);
    } else {
        return 0;
    }
}

void SCPIMM_stop_mesurement(void) {
	// TODO
}

void SCPIMM_clear_return_buffer(void) {
	// TODO
}

size_t SCPIMM_ResultDouble(scpi_t * context, double val) {
    char buffer[32];
    size_t result = 0;
    size_t len = double_to_str(buffer, val);
    result += writeDelimiter(context);
    result += writeData(context, buffer, len);
    context->output_count++;
    return result;

}

scpi_bool_t expectNoParams(scpi_t* context) {
    const char * param;
    size_t param_len;

	if (SCPI_ParamString(context, &param, &param_len, FALSE)) {
	    SCPI_ErrorPush(context, SCPI_ERROR_PARAMETER_NOT_ALLOWED);
		return FALSE;
	}

    return TRUE;
}

void signalInternalError(scpi_t* context) {
	/* TODO: signal valid error */
    SCPI_ErrorPush(context, SCPI_ERROR_UNKNOWN);
}

size_t double_to_str(char* dest, double v) {
	char* p = dest;
	int order = 6;
	unsigned long ulv;

	if (v < 0.0) {
		*p++ = '-';
		v = -v;
	} else {
		*p++ = '+';
	}

	if (v < 1.0e-38) {
		p += sprintf(p, "0.000000E+00");
	} else {
		while (!(v < 1.0e7)) {
			order++;
			v /= 10.0;
		}

		while ((v + 0.0000005) <= 1.0e6) {
			order--;
			v *= 10.0;
		}

		ulv = (unsigned long) (v + 0.5);

		p += sprintf(p, "%d", (int) (ulv / 1000000));
		*p++ = '.';
		p += sprintf(p, "%06lu", (unsigned long) (ulv % 1000000));

		*p++ = 'E';
		if (order >= 0) {
			*p++ = '+';
		} else {
			*p++ = '-';
			order = -order;
		}
		p += sprintf(p, "%02d", order);
	}

	return p - dest;
}

size_t min_value_index(const double* const values) {
	size_t result = SIZE_MAX;
	double min_value = 0.0;
	const double* v = values;

	for (; *v >= 0.0; v++) {
		if (v == values || min_value > *v) {
			min_value = *v;
			result = v - values;;
		}
	}

	return result;
}

size_t max_value_index(const double* const values) {
	size_t result = SIZE_MAX;
	double max_value = 0.0;
	const double* v = values;

	for (; *v >= 0.0; v++) {
		if (v == values || max_value < *v) {
			max_value = *v;
			result = v - values;;
		}
	}

	return result;
}

size_t less_or_equal_index(const double* values, const double value) {
	size_t result = SIZE_MAX;
	double max_value = 0.0;
	const double* v = values;

	for (; *v >= 0.0; v++) {
		if (*v <= value) {
			if (SIZE_MAX == result || max_value < *v) {
				max_value = *v;
				result = v - values;;
			}
		}
	}

	return result;
}

size_t greater_or_equal_index(const double* values, const double v) {
	size_t result;

	for (result = 0; values[result] >= 0; result++) {
		if (values[result] >= v) {
			return result;
		}
	}

	return SIZE_MAX;
}

#ifdef	ARDUINO

static scpi_bool_t iscolon(char ch) {
    return (':' == ch) ? TRUE : FALSE;
}

size_t strToDouble(const char * str, double * val) {
    char * endptr;
    *val = strtod(str, &endptr);
    return endptr - str;
}

size_t patternSeparatorShortPos(const char * pattern, size_t len) {
    size_t i;
    for (i = 0; (i < len) && pattern[i]; i++) {
        if (islower((unsigned char)pattern[i])) {
            return i;
        }
    }
    return i;
}

scpi_bool_t compareStr(const char * str1, size_t len1, const char * str2, size_t len2) {
    if (len1 != len2) {
        return FALSE;
    }

    if (SCPI_strncasecmp(str1, str2, len2) == 0) {
        return TRUE;
    }

    return FALSE;
}

scpi_bool_t matchPattern(const char * pattern, size_t pattern_len, const char * str, size_t str_len) {
    int pattern_sep_pos_short = patternSeparatorShortPos(pattern, pattern_len);
    return compareStr(pattern, pattern_len, str, str_len) ||
            compareStr(pattern, pattern_sep_pos_short, str, str_len);
}

char * strnpbrk(const char *str, size_t size, const char *set) {
    const char *scanp;
    long c, sc;
    const char * strend = str + size;

    while ((strend != str) && ((c = *str++) != 0)) {
        for (scanp = set; (sc = *scanp++) != '\0';)
            if (sc == c)
                return ((char *) (str - 1));
    }
    return (NULL);
}

size_t patternSeparatorPos(const char * pattern, size_t len) {
    
    char * separator = strnpbrk(pattern, len, "?:[]");
    if (separator == NULL) {
        return len;
    } else {
        return separator - pattern;
    }
}

size_t cmdSeparatorPos(const char * cmd, size_t len) {
    char * separator = strnpbrk(cmd, len, ":?");
    size_t result;
    if (separator == NULL) {
        result = len;
    } else {
        result = separator - cmd;
    }
    
    return result;
}

scpi_bool_t matchCommand(const char * pattern, const char * cmd, size_t len) {
    int result = FALSE;
    
    const char * pattern_ptr = pattern;
    int pattern_len = strlen(pattern);
    const char * pattern_end = pattern + pattern_len;
    
    const char * cmd_ptr = cmd;
    size_t cmd_len = SCPI_strnlen(cmd, len);
    const char * cmd_end = cmd + cmd_len;
    
    /* TODO: now it is possible to send command ":*IDN?" which is incorrect */
    if (iscolon(cmd_ptr[0])) {
        cmd_len --;
        cmd_ptr ++;
    }
    
    while (1) {
        int pattern_sep_pos = patternSeparatorPos(pattern_ptr, pattern_end - pattern_ptr);
        int cmd_sep_pos = cmdSeparatorPos(cmd_ptr, cmd_end - cmd_ptr);
        
        if (matchPattern(pattern_ptr, pattern_sep_pos, cmd_ptr, cmd_sep_pos)) {
            pattern_ptr = pattern_ptr + pattern_sep_pos;
            cmd_ptr = cmd_ptr + cmd_sep_pos;
            result = TRUE;
            
            /* command is complete */
            if ((pattern_ptr == pattern_end) && (cmd_ptr >= cmd_end)) {
                break;
            }
            
            /* pattern complete, but command not */
            if ((pattern_ptr == pattern_end) && (cmd_ptr < cmd_end)) {
                result = FALSE;
                break;
            }
            
            /* command complete, but pattern not */
            if (cmd_ptr >= cmd_end) {
                result = FALSE;
                break;
            }
            
            /* both command and patter contains command separator at this position */
            if ((pattern_ptr[0] == cmd_ptr[0]) && ((pattern_ptr[0] == ':') || (pattern_ptr[0] == '?'))) {
                pattern_ptr = pattern_ptr + 1;
                cmd_ptr = cmd_ptr + 1;
            } else {
                result = FALSE;
                break;
            }
        } else {
            result = FALSE;
            break;
        }
    }
    
    return result;
}

#endif	//	ARDUINO

