#include "fwpio.h"
#include "fwp.h"
#include "crc.h"
#include "flash.h"
#include "config.h"
#include <string.h>
#include <stdbool.h>

static UART_HandleTypeDef *uart = NULL;

void fwp_init(UART_HandleTypeDef *huart) {
    uart = huart;
}

static HAL_StatusTypeDef recv_bytes(uint8_t *buf, uint16_t length) {
    return HAL_UART_Receive(uart, buf, length, FWP_RX_TIMEOUT_MS);
}

static void send_byte(uint8_t b) {
    HAL_UART_Transmit(uart, &b, 1, HAL_MAX_DELAY);
}

static fwp_status_t recv_packet(uint8_t *type, uint16_t *seq, uint8_t *payload, uint16_t *length) {
    uint8_t b = 0;

    do {
        if (recv_bytes(&b, 1) != HAL_OK) {
            return FWP_ERR_TIMEOUT;
        }
    } while (b != FWP_SOF);

    uint8_t header[FWP_HEADER_SIZE];
    if (recv_bytes(header, FWP_HEADER_SIZE) != HAL_OK) {
        return FWP_ERR_TIMEOUT;
    }

    *type   = header[0];
    *seq    = (uint16_t) header[1] | ((uint16_t) header[2] << 8);
    *length = (uint16_t) header[3] | ((uint16_t) header[4] << 8);

    if (*length > FWP_DATA_SIZE) {
        return FWP_ERR_PROTOCOL;
    }

    if (*length > 0) {
        if (recv_bytes(payload, *length) != HAL_OK) {
            return FWP_ERR_TIMEOUT;
        }
    }

    uint8_t crc_bytes[2];
    if (recv_bytes(crc_bytes, 2) != HAL_OK) {
        return FWP_ERR_TIMEOUT;
    }
    uint16_t crc_recv = (uint16_t) crc_bytes[0] | ((uint16_t) crc_bytes[1] << 8);

    uint8_t buf[FWP_HEADER_SIZE + FWP_DATA_SIZE];
    memcpy(buf, header, FWP_HEADER_SIZE);
    if (*length > 0) {
        memcpy(&buf[FWP_HEADER_SIZE], payload, *length);
    }
    uint16_t crc_calc = crc16_modbus(buf, FWP_HEADER_SIZE + *length);

    if (crc_recv != crc_calc) {
        return FWP_ERR_CRC;
    }

    return FWP_OK;
}

fwp_status_t fwp_receive(uint8_t *dest, uint32_t *received_length) {
    if (dest == NULL || received_length == NULL) {
        return FWP_ERR_PARAM;
    }

    uint8_t payload[FWP_DATA_SIZE];
    uint8_t type = 0;
    uint16_t seq = 0;
    uint16_t length = 0;

    uint16_t expected_seq = 0;
    uint32_t total_size = 0;
    uint32_t offset = 0;
    bool started = false;

    while (1) {
        fwp_status_t st = recv_packet(&type, &seq, payload, &length);
        if (st != FWP_OK) {
            send_byte(FWP_NAK);
            continue;
        }

        if (seq != expected_seq) {
            send_byte(FWP_NAK);
            continue;
        }

        if (type == FWP_TYPE_START) {
            if (started || length != 4) {
                send_byte(FWP_NAK);
                continue;
            }
            total_size = (uint32_t) payload[0]
                       | ((uint32_t) payload[1] << 8)
                       | ((uint32_t) payload[2] << 16)
                       | ((uint32_t) payload[3] << 24);
            if (total_size == 0 || total_size > FIRMWARE_SLOT_SIZE) {
                send_byte(FWP_NAK);
                return FWP_ERR_PROTOCOL;
            }
            started = true;
            send_byte(FWP_ACK);
            expected_seq++;
        }
        else if (type == FWP_TYPE_DATA) {
            if (!started || offset + length > total_size) {
                send_byte(FWP_NAK);
                continue;
            }

            uint32_t aligned[FWP_DATA_SIZE / 4];
            memset(aligned, 0xFF, sizeof(aligned));
            memcpy(aligned, payload, length);

            uint32_t words = (length + 3) / 4;
            if (flash_write((uint32_t *) (dest + offset), aligned, words) != FLASH_OK) {
                send_byte(FWP_NAK);
                return FWP_ERR_FLASH;
            }

            offset += length;
            send_byte(FWP_ACK);
            expected_seq++;
        }
        else if (type == FWP_TYPE_END) {
            if (!started || offset != total_size) {
                send_byte(FWP_NAK);
                return FWP_ERR_PROTOCOL;
            }
            send_byte(FWP_ACK);
            *received_length = offset;
            return FWP_OK;
        }
        else {
            send_byte(FWP_NAK);
        }
    }
}
