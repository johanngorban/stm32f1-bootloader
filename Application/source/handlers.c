#include "handlers.h"
#include <string.h>

void handle_unknown_command(bcp_response_t *response) {
    response->command = BCP_UNKNOWN_COMMAND;
    response->status = BCP_ERROR_UNKNOWN_COMMAND;
    response->crc = bcp_response_calculate_crc16(response);
}

void handle_upload_firmware(const bcp_request_t *request, bcp_response_t *response) {
}

void handle_update_firmware(const bcp_request_t *request, bcp_response_t *response) {
}

void handle_calc_bank_crc(const bcp_request_t *request, bcp_response_t *response) {
}

void handle_run_firmware(const bcp_request_t *request, bcp_response_t *response) {
}

void handle_get_version(const bcp_request_t *request, bcp_response_t *response) {
}