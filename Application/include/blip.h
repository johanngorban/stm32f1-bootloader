#pragma once

#include <stdint.h>

#define BLIP_MAX_DATA_LENGTH        (255u)
#define BLIP_REQUEST_HEADER_SIZE    (2u)
#define BLIP_RESPONSE_HEADER_SIZE   (3u)

#define BLIP_SOF_BYTE (0xAAu)

typedef enum {
    BLIP_UPLOAD_FIRMWARE  = 0x01,
    BLIP_UPDATE_FIRMWARE  = 0x02,
    BLIP_CALC_BANK_CRC    = 0x03,
    BLIP_RUN_FIRMWARE     = 0x04,
    BLIP_GET_VERSION      = 0x05,
} blip_command_t;

typedef enum {
    BLIP_OK                    = 0x00,
    BLIP_ERROR_UNKNOWN_COMMAND = 0x01,
    BLIP_ERROR_INVALID_PARAM   = 0x02,
    BLIP_ERROR_BAD_CRC         = 0x03,
    BLIP_ERROR_BUSY            = 0x04,
    BLIP_ERROR_MEMORY          = 0x05,
} blip_status_t;

typedef struct {
    uint8_t command;
    uint8_t length;
    uint8_t data[BLIP_MAX_DATA_LENGTH];
    uint16_t crc;
} blip_request_t;

typedef struct {
    uint8_t command;
    uint8_t status;
    uint8_t length;
    uint8_t data[BLIP_MAX_DATA_LENGTH];
    uint16_t crc;
} blip_response_t;

uint8_t blip_request_init(blip_request_t *request);

uint8_t blip_request_parse(blip_request_t *request, const uint8_t *data, uint8_t length);

uint8_t blip_response_init(blip_response_t *response);

uint8_t blip_response_set_data(blip_response_t *response, const uint8_t *data, uint8_t length);

uint8_t blip_response_to_bytes(const blip_response_t *response, uint8_t *data);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stm32f1xx_hal.h"

void blip_uart_init(UART_HandleTypeDef *huart);

void blip_send(const blip_response_t *request);

uint8_t blip_receive(blip_request_t *response);