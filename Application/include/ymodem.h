#pragma once

#include "stm32f1xx_hal.h"
#include <stdint.h>

#define YMODEM_SOH (0x01)
#define YMODEM_STX (0x02)
#define YMODEM_EOT (0x04)
#define YMODEM_ACK (0x06)
#define YMODEM_NAK (0x15)
#define YMODEM_C   (0x43)
#define YMODEM_CAN (0x18)

#define YMODEM_HEADER_SIZE      (3)
#define YMODEM_FOOTER_SIZE      (2)
#define YMODEM_DATA_1K_SIZE     (1024)
#define YMODEM_DATA_SIZE        (128)
#define YMODEM_PACKET_MAX_SIZE  (YMODEM_HEADER_SIZE + YMODEM_DATA_1K_SIZE + YMODEM_FOOTER_SIZE)
#define YMODEM_FILENAME_MAX_LEN (64)
#define YMODEM_MAX_ATTEMPTS     (10)

typedef enum {
    YMODEM_OK = 0x00,
    YMODEM_PARAM_ERROR,
    YMODEM_MODULE_NOT_INITED,
    YMODEM_UNKNOWN_ERROR,
    YMODEM_CRC_ERROR,
    YMODEM_PROTOCOL_ERROR,
    YMODEM_CANCELLED,
    YMODEM_TIMEOUT_ERROR,
} ymodem_status_t;

/** YMODEM packet
 * ┌────────┬────────┬──────────┬─────────────────────┬──────────┐
 * │ START  │  NUM   │  ~NUM    │        DATA         │  CRC-16  │
 * │ 1 byte │ 1 byte │  1 byte  │   128 or 1024 bytes │  2 bytes │
 * └────────┴────────┴──────────┴─────────────────────┴──────────┘
 * If num = 0x00 (the very first packet), data is always 128 bytes and contains:
 * ┌──────────────────────────┬─────┬───────────────────────┬─────┐
 * │ filename (ASCII, \0)     │ \0  │ file size (ASCII, \0) │  0  │
 * └──────────────────────────┴─────┴───────────────────────┴─────┘
 */

uint8_t ymodem_init(const UART_HandleTypeDef *uart);

ymodem_status_t ymodem_receive(uint8_t *data, uint32_t *length, char *filename);

ymodem_status_t ymodem_transmit(uint8_t *data, uint32_t length, const char *filename);
