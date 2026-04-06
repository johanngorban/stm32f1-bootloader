#pragma once

#include "bcp.h"

void handle_unknown_command(bcp_response_t *response);

void handle_upload_firmware(const bcp_request_t *request, bcp_response_t *response);

void handle_update_firmware(const bcp_request_t *request, bcp_response_t *response);

void handle_calc_bank_crc(const bcp_request_t *request, bcp_response_t *response);

void handle_run_firmware(const bcp_request_t *request, bcp_response_t *response);

void handle_get_version(const bcp_request_t *request, bcp_response_t *response);