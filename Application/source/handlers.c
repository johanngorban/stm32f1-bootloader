#include "handlers.h"
#include "config.h"
#include "flash.h"
#include "fwp_receive.h"
#include "crc.h"
#include "bcp_io.h"
#include <string.h>

void handle_unknown_command(bcp_response_t *response) {
    response->command = BCP_UNKNOWN_COMMAND;
    response->status = BCP_ERROR_UNKNOWN_COMMAND;
    response->crc = bcp_response_calculate_crc16(response);
    bcp_send_response(response);
}

void handle_upload_firmware(const bcp_request_t *request, bcp_response_t *response) {
    uint8_t page_start = (FIRMWARE_BANK2_START - FLASH_BASE) / FLASH_PAGE_SIZE;
    uint8_t page_count = FIRMWARE_BANK_SIZE / FLASH_PAGE_SIZE;

    flash_status_t flash_status = flash_erase(page_start, page_count);
    if (flash_status != FLASH_OK) {
        response->status = BCP_ERROR_MEMORY;
        response->crc = bcp_response_calculate_crc16(response);
        bcp_send_response(response);
        return;
    }

    response->crc = bcp_response_calculate_crc16(response);
    bcp_send_response(response);

    uint32_t length = 0;
    fwp_receive((uint8_t *) FIRMWARE_BANK2_START, &length);
}

void handle_update_firmware(const bcp_request_t *request, bcp_response_t *response) {
}

void handle_calc_bank_crc(const bcp_request_t *request, bcp_response_t *response) {
    if (request->length != 1) {
        response->status = BCP_ERROR_INVALID_PARAM;
        return;
    }

    uint8_t bank_num = request->data[0];
    uint16_t crc = 0;
    switch (bank_num) {
    case 1:
        crc = crc16_calculate((const uint8_t *) FIRMWARE_BANK1_START, FIRMWARE_BANK_SIZE);
        break;
    case 2:
        crc = crc16_calculate((const uint8_t *) FIRMWARE_BANK2_START, FIRMWARE_BANK_SIZE);
        break;
    default:
        response->status = BCP_ERROR_INVALID_PARAM;
        return;
    }

    response->data[0] = (crc >> 8) & 0xFF;
    response->data[1] = crc & 0xFF;
    response->length = 2;

    response->crc = bcp_response_calculate_crc16(response);
    bcp_send_response(response);
}

void handle_run_firmware(const bcp_request_t *request, bcp_response_t *response) {
}

void handle_get_version(const bcp_request_t *request, bcp_response_t *response) {
    response->command = request->command;
    response->status = BCP_OK;

    response->data[0] = BOOTLOADER_MAJOR_VERSION;
    response->data[1] = BOOTLOADER_MINOR_VERSION;
    response->data[2] = BOOTLOADER_PATCH_VERSION;
    response->length = 3;

    response->crc = bcp_response_calculate_crc16(response);
    bcp_send_response(response);
}
