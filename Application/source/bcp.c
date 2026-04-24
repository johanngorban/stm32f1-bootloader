#include "bcp.h"
#include "crc.h"
#include <string.h>

uint8_t bcp_request_init(bcp_request_t *request) {
    if (request == NULL) {
        return 1;
    }
    memset(request, 0, sizeof(bcp_request_t));
    return 0;
}

uint8_t bcp_response_init(bcp_response_t *response) {
    if (response == NULL) {
        return 1;
    }
    memset(response, 0, sizeof(bcp_response_t));
    response->status = BCP_OK;

    return 0;
}

inline uint16_t bcp_request_calculate_crc16(const bcp_request_t *request) {
    return crc16_modbus((const uint8_t *) request, request->length + BCP_REQUEST_HEADER_SIZE);
}

inline uint16_t bcp_response_calculate_crc16(const bcp_response_t *response) {
    return crc16_modbus((const uint8_t *) response, response->length + BCP_RESPONSE_HEADER_SIZE);
}
