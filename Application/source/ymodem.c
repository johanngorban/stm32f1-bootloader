#include "ymodem.h"
#include "crc.h"
#include "stm32f1xx_hal.h"
#include "flash.h"
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static UART_HandleTypeDef *ymodem_uart = NULL;

static bool ymodem_inited = false;

static const uint8_t ack = YMODEM_ACK;
static const uint8_t nak = YMODEM_NAK;

static uint8_t packet_data[YMODEM_DATA_1K_SIZE];

uint8_t ymodem_init(const UART_HandleTypeDef *uart) {
    if (ymodem_inited == true) {
        return 1;
    }

    ymodem_uart = uart;
    ymodem_inited = true;
    return 0;
}

/**
 * Example
 * HAL_UART_Receive(ymodem_uart, uint8_t *data, uint32_t length, void *ctx) {
 *  huart = (UART_TypeDef *) ctx;
 *  HAL_UART_Receive()
 *  if (IS_FLASH(data)) {
 *      flash_write(data, length);
 *  } else {
 *
 *  }
 *  }
 * }
 */

ymodem_status_t ymodem_receive(uint8_t *addr, uint32_t *length, char *filename) {
    if (addr == NULL || length == NULL) {
        return YMODEM_PARAM_ERROR;
    }
    if (ymodem_inited == false) {
        return YMODEM_MODULE_NOT_INITED;
    }

    memset(packet_data, 0, YMODEM_DATA_1K_SIZE);

    const uint8_t C = 'C';
    HAL_UART_Transmit(ymodem_uart, &C, 1, HAL_MAX_DELAY);

    uint8_t byte = 0;
    HAL_UART_Receive(ymodem_uart, &byte, 1, HAL_MAX_DELAY);
    if (byte != YMODEM_SOH) {
        return YMODEM_PROTOCOL_ERROR;
    }

    uint8_t block_n = 0;
    uint8_t block_n_inv = 0;
    HAL_UART_Receive(ymodem_uart, &block_n, 1, HAL_MAX_DELAY);
    HAL_UART_Receive(ymodem_uart, &block_n_inv, 1, HAL_MAX_DELAY);

    if (block_n != 0x00 || (uint8_t) (block_n + block_n_inv) != 0xFF) {
        HAL_UART_Transmit(ymodem_uart, &nak, 1, HAL_MAX_DELAY);
        return YMODEM_PROTOCOL_ERROR;
    }

    memset(packet_data, 0, YMODEM_DATA_1K_SIZE);
    HAL_UART_Receive(ymodem_uart, packet_data, YMODEM_DATA_SIZE, HAL_MAX_DELAY);

    uint16_t crc = 0;
    HAL_UART_Receive(ymodem_uart, (uint8_t *) &crc, 2, HAL_MAX_DELAY);

    if (crc != crc16_calculate(packet_data, YMODEM_DATA_SIZE)) {
        HAL_UART_Transmit(ymodem_uart, &nak, 1, HAL_MAX_DELAY);
        return YMODEM_CRC_ERROR;
    }

    if (filename != NULL) {
        strncpy(filename, (char *) packet_data, YMODEM_FILENAME_MAX_LEN - 1);
    }

    const char *size_ptr = (char *) packet_data + strlen((char *) packet_data) + 1;
    uint32_t file_size = atoi(size_ptr);
    *length = file_size;

    HAL_UART_Transmit(ymodem_uart, &ack, 1, HAL_MAX_DELAY);
    HAL_UART_Transmit(ymodem_uart, &C, 1, HAL_MAX_DELAY);

    uint8_t expected_block = 1;
    uint32_t packet_size = 0;
    uint32_t total_written = 0;
    while (1) {
        HAL_UART_Receive(ymodem_uart, &byte, 1, HAL_MAX_DELAY);
        if (byte == YMODEM_EOT) {
            HAL_UART_Transmit(ymodem_uart, &nak, 1, HAL_MAX_DELAY);
            HAL_UART_Receive(ymodem_uart, &byte, 1, HAL_MAX_DELAY);
            if (byte != YMODEM_EOT) {
                return YMODEM_PROTOCOL_ERROR;
            }
            HAL_UART_Transmit(ymodem_uart, &ack, 1, HAL_MAX_DELAY);
            HAL_UART_Transmit(ymodem_uart, &C, 1, HAL_MAX_DELAY);
            break;
        }

        if (byte == YMODEM_SOH) {
            packet_size = YMODEM_DATA_SIZE;
        } else if (byte == YMODEM_STX) {
            packet_size = YMODEM_DATA_1K_SIZE;
        } else {
            HAL_UART_Transmit(ymodem_uart, &nak, 1, HAL_MAX_DELAY);
            continue;
        }

        HAL_UART_Receive(ymodem_uart, &block_n, 1, HAL_MAX_DELAY);
        HAL_UART_Receive(ymodem_uart, &block_n_inv, 1, HAL_MAX_DELAY);

        if ((uint8_t) (block_n + block_n_inv) != 0xFF) {
            HAL_UART_Transmit(ymodem_uart, &nak, 1, HAL_MAX_DELAY);
            continue;
        }

        if (block_n != (uint8_t) (expected_block & 0xFF)) {
            HAL_UART_Transmit(ymodem_uart, &nak, 1, HAL_MAX_DELAY);
            continue;
        }

        HAL_UART_Receive(ymodem_uart, packet_data, packet_size, HAL_MAX_DELAY);

        HAL_UART_Receive(ymodem_uart, (uint8_t *) &crc, 2, HAL_MAX_DELAY);
        if (crc != crc16_calculate(addr, YMODEM_DATA_SIZE)) {
            HAL_UART_Transmit(ymodem_uart, &nak, 1, HAL_MAX_DELAY);
            continue;
        }

        uint32_t to_copy = packet_size;
        if (file_size > 0 && total_written + to_copy > file_size) {
            to_copy = file_size - total_written;
        }

        flash_write((uint32_t *) (addr + total_written), (uint32_t *) packet_data, to_copy);
        total_written += to_copy;
        expected_block++;

        HAL_UART_Transmit(ymodem_uart, &ack, 1, HAL_MAX_DELAY);
    }

    return YMODEM_OK;
}

ymodem_status_t ymodem_transmit(uint8_t *data, uint32_t length, const char *filename) {
    if (ymodem_inited == false) {
        return YMODEM_MODULE_NOT_INITED;
    }

    memset(packet_data, 0, YMODEM_DATA_1K_SIZE);

    uint8_t byte = 0;
    HAL_UART_Receive(ymodem_uart, &byte, 1, HAL_MAX_DELAY);
    if (byte != YMODEM_C) {
        return YMODEM_PROTOCOL_ERROR;
    }

    uint8_t header[3] = {YMODEM_SOH, 0x00, 0xFF};
    uint16_t data_offset = 0;
    if (filename != NULL) {
        snprintf((char *) packet_data, YMODEM_DATA_SIZE, "%s", filename);
        data_offset = strlen(filename) + 1;
    }
    snprintf((char *) (packet_data + data_offset), YMODEM_DATA_SIZE - data_offset, "%lu", length);
    HAL_UART_Transmit(ymodem_uart, header, 3, HAL_MAX_DELAY);
    HAL_UART_Transmit(ymodem_uart, packet_data, YMODEM_DATA_SIZE, HAL_MAX_DELAY);

    uint16_t crc = crc16_calculate(packet_data, YMODEM_DATA_SIZE);
    HAL_UART_Transmit(ymodem_uart, (const uint8_t *) &crc, 2, HAL_MAX_DELAY);

    HAL_UART_Receive(ymodem_uart, &byte, 1, HAL_MAX_DELAY);
    if (byte != YMODEM_ACK) {
        return YMODEM_PROTOCOL_ERROR;
    }
    HAL_UART_Receive(ymodem_uart, &byte, 1, HAL_MAX_DELAY);
    if (byte != YMODEM_C) {
        return YMODEM_PROTOCOL_ERROR;
    }

    uint8_t block_n = 1;
    data_offset = 0;
    uint32_t remaining = length;

    while (remaining > 0) {
        uint32_t packet_size = (remaining >= YMODEM_DATA_1K_SIZE) ? YMODEM_DATA_1K_SIZE : YMODEM_DATA_SIZE;
        uint8_t soh = (packet_size == YMODEM_DATA_SIZE) ? YMODEM_SOH : YMODEM_STX;

        memset(packet_data, 0x1A, packet_size);
        uint32_t to_copy = (remaining < packet_size) ? remaining : packet_size;
        memcpy(packet_data, data + data_offset, to_copy);

        crc = crc16_calculate(packet_data, packet_size);

        uint8_t attempts = 0;
        bool packet_acked = false;

        while (attempts < YMODEM_MAX_ATTEMPTS) {
            header[0] = soh;
            header[1] = block_n;
            header[2] = (uint8_t) (0xFF - block_n);
            HAL_UART_Transmit(ymodem_uart, header, 3, HAL_MAX_DELAY);
            HAL_UART_Transmit(ymodem_uart, packet_data, packet_size, HAL_MAX_DELAY);
            HAL_UART_Transmit(ymodem_uart, (const uint8_t *) &crc, 2, HAL_MAX_DELAY);

            HAL_UART_Receive(ymodem_uart, &byte, 1, HAL_MAX_DELAY);
            if (byte == YMODEM_ACK) {
                packet_acked = true;
                break;
            } else if (byte == YMODEM_NAK) {
                attempts++;
            } else if (byte == YMODEM_CAN) {
                return YMODEM_CANCELLED;
            } else {
                attempts++;
            }
        }

        if (packet_acked == false) {
            uint8_t can[2] = {YMODEM_CAN, YMODEM_CAN};
            HAL_UART_Transmit(ymodem_uart, can, 2, HAL_MAX_DELAY);
            return YMODEM_TIMEOUT_ERROR;
        }

        data_offset += to_copy;
        remaining -= to_copy;
        block_n++;
    }

    uint8_t eot = YMODEM_EOT;
    HAL_UART_Transmit(ymodem_uart, &eot, 1, HAL_MAX_DELAY);
    HAL_UART_Receive(ymodem_uart, &byte, 1, HAL_MAX_DELAY);

    HAL_UART_Transmit(ymodem_uart, &eot, 1, HAL_MAX_DELAY);
    HAL_UART_Receive(ymodem_uart, &byte, 1, HAL_MAX_DELAY);
    HAL_UART_Receive(ymodem_uart, &byte, 1, HAL_MAX_DELAY);

    return YMODEM_OK;
}
