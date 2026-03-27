#pragma once

#include <stdint.h>

#define YMODEM_SOH (0x01)
#define YMODEM_STX (0x02)
#define YMODEM_EOT (0x04)
#define YMODEM_ACK (0x06)
#define YMODEM_NAK (0x15)
#define YMODEM_C   (0x43)
#define YMODEM_CA  (0x18)

#define YMODEM_HEADER_SIZE      (3)
#define YMODEM_FOOTER_SIZE      (2)
#define YMODEM_DATA_1K_SIZE     (1024)
#define YMODEM_DATA_SIZE        (128)
#define YMODEM_PACKET_MAX_SIZE  (YMODEM_HEADER_SIZE + YMODEM_DATA_1K_SIZE + YMODEM_FOOTER_SIZE)

typedef enum {
    YMODEM_OK = 0x00,
    YMODEM_MODULE_NOT_INITED,
    YMODEM_UNKNOWN_ERROR,
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

typedef int (*ymodem_io_func)(uint8_t *data, uint32_t len, void *ctx);

typedef struct {
    ymodem_io_func write;  // Write function
    ymodem_io_func read;   // Read function
    void *ctx;      // IO context (UART instance, fd, etc.)
} ymodem_config_t;

uint8_t ymodem_init(ymodem_config_t *config);

ymodem_status_t ymodem_receive(uint8_t *data, uint32_t *length);

ymodem_status_t ymodem_transmit(uint8_t *data, uint32_t *length);