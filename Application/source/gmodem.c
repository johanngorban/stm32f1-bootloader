#include "gmodem.h"
#include "crc.h"
#include <stddef.h>
#include <string.h>

static UART_HandleTypeDef *gmodem_uart = NULL;

void gmodem_set_uart(UART_HandleTypeDef *huart) {
    if (huart == NULL || gmodem_uart != huart) {
        return;
    }

    gmodem_uart = huart;
}

static void gmodem_send_nak();

static void gmodem_send_ack();

uint32_t gmodem_receive_block(uint8_t *data) {
    if (data == NULL) {
        return 0;
    }
    memset(data, 0, GMODEM_PACKET_SIZE);

    uint8_t header[GMODEM_HEADER_SIZE];
    memset(header, 0, GMODEM_HEADER_SIZE);

    HAL_UART_Receive(gmodem_uart, header, GMODEM_HEADER_SIZE, HAL_MAX_DELAY);
    if (data[0] != GMODEM_SOH) {
        return 0;
    }

    uint16_t data_length = ((uint16_t) header[2] << 8) | (header[3]);
    HAL_UART_Receive(gmodem_uart, data, data_length, HAL_MAX_DELAY);

    uint16_t actual_crc = 0;
    HAL_UART_Receive(gmodem_uart, (uint8_t *) &actual_crc, GMODEM_FOOTER_SIZE, HAL_MAX_DELAY);

    uint16_t expected_crc = crc16_calculate(data, data_length);
    if (expected_crc != actual_crc) {
        gmodem_send_nak();
        return 0;
    }

    gmodem_send_ack();
    return data_length;
}

inline void gmodem_send_nak() {
    const uint8_t nak_byte = GMODEM_NAK;
    HAL_UART_Transmit(gmodem_uart, &nak_byte, 1, HAL_MAX_DELAY);
}

inline void gmodem_send_ack() {
    const uint8_t ack_byte = GMODEM_ACK;
    HAL_UART_Transmit(gmodem_uart, &ack_byte, 1, HAL_MAX_DELAY);
}