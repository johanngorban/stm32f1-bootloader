#include "router.h"
#include "handlers.h"
#include "bcp_io.h"
#include <stddef.h>
#include <assert.h>

static const router_entry_t router[] = {
    {BCP_VERSION, handle_get_version},
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

static void send_error(const bcp_request_t *request, bcp_status_t status) {
    bcp_response_t response;
    bcp_response_init(&response);

    response.command = request->command;
    response.status = status;
    response.crc = bcp_response_calculate_crc16(&response);

    bcp_send_response(&response);
}

void router_handle_request(const bcp_request_t *request) {
    assert(request != NULL);

    uint16_t crc16 = bcp_request_calculate_crc16(request);
    if (crc16 != request->crc) {
        send_error(request, BCP_ERROR_BAD_CRC);
        return;
    }

    bcp_response_t response;
    bcp_response_init(&response);

    handler_t handler = find_handler(request->command);
    if (handler == NULL) {
        handle_unknown_command(request, &response);
    } else {
        response.command = request->command;
        handler(request, &response);
        response.crc = bcp_response_calculate_crc16(&response);
    }

    bcp_send_response(&response);
}
