#include "handlers.h"
#include "config.h"
#include "flash.h"
#include "fwpio.h"
#include "crc.h"
#include "bcp_io.h"
#include <string.h>

void handle_unknown_command(bcp_response_t *response) {
    response->command = BCP_UNKNOWN_COMMAND;
    response->status = BCP_ERROR_UNKNOWN_COMMAND;
    response->crc = bcp_response_calculate_crc16(response);
    bcp_send_response(response);
}
