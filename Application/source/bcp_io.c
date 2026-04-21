#include "bcp_io.h"

#include <string.h>

#define BCP_RECV_MAX_ATTEMPTS   (5)

static  UART_HandleTypeDef *uart = NULL;

void bcp_uart_init(UART_HandleTypeDef *huart) {
    if (uart != NULL) {
        return;
    }

    uart = huart;
}

int8_t bcp_send_response(const bcp_response_t *response) {
    uint16_t packet_length = 1 + BCP_RESPONSE_HEADER_SIZE + response->length + 2;
    uint8_t packet[packet_length];

    packet[0] = BCP_SOF_BYTE;
    packet[1] = response->command;
    packet[2] = response->status;
    packet[3] = response->length;
    memcpy(&packet[4], response->data, response->length);
    memcpy(&packet[4 + response->length], &response->crc, 2);

    HAL_UART_Transmit(uart, packet, packet_length, HAL_MAX_DELAY);

    return 0;
}

int8_t bcp_recv_request(bcp_request_t *request) {
    uint8_t sof_byte = 0;
    do {
        HAL_UART_Receive(uart, &sof_byte, 1, HAL_MAX_DELAY);
    } while (sof_byte != BCP_SOF_BYTE);

    HAL_UART_Receive(uart, &request->command, 1, HAL_MAX_DELAY);
    HAL_UART_Receive(uart, &request->length, 1, HAL_MAX_DELAY);
    HAL_UART_Receive(uart, request->data, request->length, HAL_MAX_DELAY);
    HAL_UART_Receive(uart, (uint8_t *) &request->crc, 2, HAL_MAX_DELAY);

    return BCP_RECV_OK;
}
