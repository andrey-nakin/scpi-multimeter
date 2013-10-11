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

void signalInternalError(scpi_t* context) {
	/* TODO: signal valid error */
    SCPI_ErrorPush(context, SCPI_ERROR_INTERNAL);
}

#ifdef	ARDUINO

static bool_t iscolon(char ch) {
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

bool_t matchCommand(const char * pattern, const char * cmd, size_t len) {
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

