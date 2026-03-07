#include "handlers.h"
#include <string.h>

void handle_request(const blip_request_t *request, blip_response_t *response) {
    blip_response_init(response);
    response->command = request->command;
    response->status = BLIP_OK;
    response->length = BLIP_MAX_DATA_LENGTH;
    response->crc = 0xAA;
    memset(response->data, 1, response->length);
}