#pragma once

#include "bcp.h"

void handle_unknown_command(const bcp_request_t *request, bcp_response_t *response);

void handle_get_version(const bcp_request_t *request, bcp_response_t *response);
