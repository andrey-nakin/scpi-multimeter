#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <scpi/config.h>
#include "utils.h"

size_t strToDouble(const char * str, double * val);
size_t patternSeparatorShortPos(const char * pattern, size_t len);
bool_t compareStr(const char * str1, size_t len1, const char * str2, size_t len2);
bool_t matchPattern(const char * pattern, size_t pattern_len, const char * str, size_t str_len);


void SCPIMM_stop_mesurement(void) {
	// TODO
}

void SCPIMM_clear_return_buffer(void) {
	// TODO
}

bool_t expectNoParams(scpi_t* context) {
    const char * param;
    size_t param_len;

	if (SCPI_ParamString(context, &param, &param_len, FALSE)) {
	    SCPI_ErrorPush(context, SCPI_ERROR_PARAMETER_NOT_ALLOWED);
		return FALSE;
	}

    return TRUE;
}

/*
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

bool_t compareStr(const char * str1, size_t len1, const char * str2, size_t len2) {
    if (len1 != len2) {
        return FALSE;
    }

    if (SCPI_strncasecmp(str1, str2, len2) == 0) {
        return TRUE;
    }

    return FALSE;
}

bool_t matchPattern(const char * pattern, size_t pattern_len, const char * str, size_t str_len) {
    int pattern_sep_pos_short = patternSeparatorShortPos(pattern, pattern_len);
    return compareStr(pattern, pattern_len, str, str_len) ||
            compareStr(pattern, pattern_sep_pos_short, str, str_len);
}
*/
