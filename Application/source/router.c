#include "router.h"
#include "handlers.h"

static const router_entry_t router[] = {
    {BCP_UPLOAD_FIRMWARE, handle_upload_firmware},
    {BCP_UPDATE_FIRMWARE, handle_update_firmware},
    {BCP_CALC_BANK_CRC, handle_calc_bank_crc},
    {BCP_RUN_FIRMWARE, handle_run_firmware},
    {BCP_GET_VERSION, handle_get_version},
};

static handler_t find_handler(bcp_command_t id) {
    uint16_t length = sizeof(router) / sizeof(router_entry_t);
    
    handler_t handler = NULL;
    while (length--) {
        if (router[length - 1].id == id) {
            handler = router[length - 1].handler;
            break;
        }
    }

    return handler;
}

static void send_error(bcp_command_t cmd, bcp_status_t status) {
    bcp_response_t response;
    bcp_response_init(&response);

    response.command = cmd;
    response.status = status;
    // bcp_calculate_crc16(&response);

    bcp_send(&response);
}

static void send_stub(const bcp_request_t *request) {
    bcp_response_t response;
    bcp_response_init(&response);

    response.command = request->command;
    response.status = BCP_OK;
}

void router_handle_request(const bcp_request_t *request) {
    if (request == NULL) {
        send_error(BCP_UNKNOWN_COMMAND, BCP_ERROR_INVALID_PARAM);
        return;
    }

    uint16_t crc16 = bcp_request_calculate_crc16(request);
    if (crc16 != request->crc) {
        send_error(request->command, BCP_ERROR_BAD_CRC);
        return;
    }

    bcp_response_t response;
    bcp_response_init(&response);

    handler_t handler = find_handler(request->command);
    if (handler == NULL) {
        send_error(request->command, BCP_ERROR_UNKNOWN_COMMAND);
        return;
    }
    // handler(request, &response);
    send_stub(request);

    bcp_send(&response);
}