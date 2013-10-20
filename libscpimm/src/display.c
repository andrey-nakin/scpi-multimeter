#include <string.h>
#include "display.h"
#include "utils.h"

scpi_result_t SCPIMM_display(scpi_t* context) {
	bool_t b;

    if (!SCPI_ParamBool(context, &b, TRUE)) {
		return SCPI_RES_ERR;
	}

	SCPIMM_CONTEXT(context)->display = b;
    return SCPI_RES_OK;
}

scpi_result_t SCPIMM_displayQ(scpi_t* context) {
	SCPI_ResultBool(context, SCPIMM_CONTEXT(context)->display);
    return SCPI_RES_OK;
}

scpi_result_t SCPIMM_display_text(scpi_t* context) {
	scpimm_context_t* const ctx = SCPIMM_CONTEXT(context);
	const char* text;
	size_t text_len;

	if (!SCPI_ParamText(context, &text, &text_len, TRUE)) {
		return SCPI_RES_ERR;
	}

	text_len -= 2;
	if (text_len > SCPIMM_DISPLAY_LEN) {
	    SCPI_ErrorPush(context, SCPI_ERROR_TOO_MUCH_DATA);
    	return SCPI_RES_ERR;
	}

	strncpy(ctx->display_text, text + 1, text_len);
	ctx->display_text[text_len] = '\0';
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_display_textQ(scpi_t* context) {
	SCPI_ResultText(context, SCPIMM_CONTEXT(context)->display_text);
	return SCPI_RES_OK;
}

scpi_result_t SCPIMM_display_text_clear(scpi_t* context) {
	if (expectNoParams(context)) {
		SCPIMM_CONTEXT(context)->display_text[0] = '\0';
	}
	return SCPI_RES_OK;
}

