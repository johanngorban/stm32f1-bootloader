#include "router.h"
#include "handlers.h"
#include "bcp_io.h"
#include <stddef.h>

static const router_entry_t router[] = {
    {BCP_VERSION, handle_get_version},
    {BCP_FLASH, handle_flash},
    {BCP_VERIFY, handle_verify},
    {BCP_RUN, handle_run},
};

static handler_t find_handler(bcp_command_t id) {
    uint16_t length = sizeof(router) / sizeof(router_entry_t);

    handler_t handler = NULL;
    for (uint16_t i = 0; i < length; i++) {
        if (router[i].id == id) {
            handler = router[i].handler;
            break;
        }
    }

    return handler;
}

void router_handle_request(const bcp_request_t *request) {
    bcp_response_t response;
    bcp_response_init(&response);
    response.command = request->command;

    if (bcp_request_calculate_crc16(request) != request->crc) {
        response.status = BCP_ERROR_BAD_CRC;
    } else {
        handler_t handler = find_handler(request->command);
        if (handler == NULL) {
            response.status = BCP_ERROR_UNKNOWN_COMMAND;
        } else {
            handler(request, &response);
        }
    }

    bcp_send_response(&response);
    if (response.post_callback) {
        response.post_callback(response.post_callback_arg);
    }
}
