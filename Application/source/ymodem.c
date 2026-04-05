#include "ymodem.h"
#include "crc.h"
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define YMODEM_MAX_ATTEMPTS (5)

static ymodem_config_t config = {};
static bool ymodem_inited = false;

static const uint8_t ack = YMODEM_ACK;
static const uint8_t nak = YMODEM_NAK;
static const uint8_t c   = YMODEM_C;

static uint8_t packet_data[YMODEM_DATA_1K_SIZE];

uint8_t ymodem_init(ymodem_config_t *cfg) {
    if (cfg == NULL || ymodem_inited == true) {
        return 1;
    }

    if (cfg->ctx == NULL) {
        return 1;
    }

    if (cfg->read == NULL) {
        return 1;
    }

    if (cfg->write == NULL) {
        return 1;
    }

    config = *cfg;
    ymodem_inited = true;
    return 0;
}

/**
 * Example
 * read(uint8_t *data, uint32_t length, void *ctx) {
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

ymodem_status_t ymodem_receive(uint8_t *data, uint32_t *length, char *filename) {
    if (data == NULL || length == NULL) {
        return YMODEM_PARAM_ERROR;
    }
    if (ymodem_inited == false) {
        return YMODEM_MODULE_NOT_INITED;
    }

    memset(packet_data, 0, YMODEM_DATA_1K_SIZE);

    ymodem_io_func read = config.read;
    ymodem_io_func write = config.write;

    const uint8_t C = 'C';
    write(&C, 1, config.ctx);

    uint8_t byte = 0;
    read(&byte, 1, config.ctx);
    if (byte != YMODEM_SOH) {
        return YMODEM_PROTOCOL_ERROR;
    }

    uint8_t block_n = 0;
    uint8_t block_n_inv = 0;
    read(&block_n, 1, config.ctx);
    read(&block_n_inv, 1, config.ctx);

    if (block_n != 0x00 || (uint8_t) (block_n + block_n_inv) != 0xFF) {
        write(&nak, 1, config.ctx);
        return YMODEM_PROTOCOL_ERROR;
    }

    memset(packet_data, 0, YMODEM_DATA_1K_SIZE);
    read(packet_data, YMODEM_DATA_SIZE, config.ctx);

    uint16_t crc = 0;
    read((uint8_t *) &crc, 2, config.ctx);

    if (crc != crc16_calculate(data, YMODEM_DATA_SIZE)) {
        write(&nak, 1, config.ctx);
        return YMODEM_CRC_ERROR;
    }

    if (filename != NULL) {
        strncpy(filename, (char *) packet_data, YMODEM_FILENAME_MAX_LEN - 1);
    }

    const char *size_ptr = (char *) packet_data + strlen((char *) packet_data) + 1;
    uint32_t file_size = atoi(size_ptr);
    *length = file_size;

    write(&ack, 1, config.ctx);
    write(&C, 1, config.ctx);

    uint8_t expected_block = 1;
    uint32_t packet_size = 0;
    uint32_t total_written = 0;
    while (1) {
        read(&byte, 1, config.ctx);
        if (byte == YMODEM_EOT) {
            write(&nak, 1, config.ctx);
            read(&byte, 1, config.ctx);
            if (byte != YMODEM_EOT) {
                return YMODEM_PROTOCOL_ERROR;
            }
            write(&ack, 1, config.ctx);
            write(&C, 1, config.ctx);
            break;
        }

        if (byte == YMODEM_SOH) {
            packet_size = YMODEM_DATA_SIZE;
        } else if (byte == YMODEM_STX) {
            packet_size = YMODEM_DATA_1K_SIZE;
        } else {
            write(&nak, 1, config.ctx);
            continue;
        }

        read(&block_n, 1, config.ctx);
        read(&block_n_inv, 1, config.ctx);

        if ((uint8_t) (block_n + block_n_inv) != 0xFF) {
            write(&nak, 1, config.ctx);
            continue;
        }

        if (block_n != (uint8_t) (expected_block & 0xFF)) {
            write(&nak, 1, config.ctx);
            continue;
        }

        read(packet_data, packet_size, config.ctx);

        read((uint8_t *) &crc, 2, config.ctx);
        if (crc != crc16_calculate(data, YMODEM_DATA_SIZE)) {
            write(&nak, 1, config.ctx);
            continue;
        }

        uint32_t to_copy = packet_size;
        if (file_size > 0 && total_written + to_copy > file_size) {
            to_copy = file_size - total_written;
        }

        memcpy(data + total_written, packet_data, to_copy);
        total_written += to_copy;
        expected_block++;

        write(&nak, 1, config.ctx);
    }

    return YMODEM_OK;
}

ymodem_status_t ymodem_transmit(uint8_t *data, uint32_t length, const char *filename) {
    if (ymodem_inited == false) {
        return YMODEM_MODULE_NOT_INITED;
    }

    ymodem_io_func write = config.write;
    ymodem_io_func read = config.read;

    memset(packet_data, 0, YMODEM_DATA_1K_SIZE);

    uint8_t byte = 0;
    read(&byte, 1, config.ctx);
    if (byte != YMODEM_C) {
        return YMODEM_PROTOCOL_ERROR;
    }

    uint8_t header[3] = {YMODEM_SOH, 0x00, 0xFF};
    uint16_t data_offset = 0;
    if (filename != NULL) {
        snprintf((char *) packet_data, YMODEM_DATA_SIZE, "%s", filename);
        data_offset = strlen(filename) + 1;
    }
    snprintf((char *) (packet_data + data_offset), YMODEM_DATA_SIZE - data_offset, "%ul", length);
    write(header, 3, config.ctx);
    write(packet_data, YMODEM_DATA_SIZE, config.ctx);

    uint16_t crc = crc16_calculate(packet_data, YMODEM_DATA_SIZE);
    write((const uint8_t *) &crc, 2, config.ctx);

    read(&byte, 1, config.ctx);
    if (byte != YMODEM_ACK) {
        return YMODEM_PROTOCOL_ERROR;
    }
    read(&byte, 1, config.ctx);
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
            header = {soh, block_n, (uint8_t) (0xFF - block_n)};
            write(header, 3, config.ctx);
            write(packet_data, packet_size, config.ctx);
            write((const uint8_t *) &crc, 2, config.ctx);

            read(&byte, 1, config.ctx);
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
            write(can, 2, config.ctx);
            return YMODEM_TIMEOUT_ERROR;
        }

        data_offset += to_copy;
        remaining -= to_copy;
        block_n++;
    }

    uint8_t eot = YMODEM_EOT;
    write(&eot, 1, config.ctx);
    read(&byte, 1, config.ctx);

    write(&eot, 1, config.ctx);
    read(&byte, 1, config.ctx);
    read(&byte, 1, config.ctx);

    return YMODEM_OK;
}
