#include "handlers.h"
#include "config.h"
#include "flash.h"
#include "fwpio.h"
#include "crc.h"
#include "bcp_io.h"
#include "image.h"
#include <string.h>

void handle_unknown_command(const bcp_request_t *request, bcp_response_t *response) {
    response->command = request->command;
    response->status = BCP_ERROR_UNKNOWN_COMMAND;
    response->crc = bcp_response_calculate_crc16(response);
    bcp_send_response(response);
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

void handle_flash(const bcp_request_t *request, bcp_response_t *response) {

}

void handle_run(const bcp_request_t *request, bcp_response_t *response) {

}

void handle_verify(const bcp_request_t *request, bcp_response_t *response) {
    uint8_t slot = request->data[0];
    if ((slot < 1) || (slot > 2)) {
        response->status = BCP_ERROR_INVALID_SLOT;
        return;
    }

    const uint32_t *image_addr = FIRMWARE_SLOT_1_START + FIRMWARE_SLOT_SIZE * (slot - 1);

    const image_metadata_t *metadata = (const image_metadata_t *) image_addr;
    if (metadata == NULL) {
        response->status = BCP_ERROR_INTERNAL_ERROR;
        return;
    }

    if (metadata->magic != IMAGE_MAGIC_NUMBER) {
        response->data[0] = 0;
        return;
    }

    if (metadata->size == 0) {
        response->data[0] = 0;
        return;
    }

    if ((metadata->size + IMAGE_METADATA_SIZE) > FIRMWARE_SLOT_SIZE) {
        response->data[0] = 0;
        return;
    }

    const uint32_t *image_body_addr = image_addr + IMAGE_METADATA_SIZE;
    uint32_t image_body_crc = crc32_iso_hdlc((const uint8_t *) image_body_addr, metadata->size);
    if (image_body_crc != metadata->crc) {
        response->data[0] = 0;
        return;
    }

    // valid marker
    response->data[0] = 1;

    // version
    response->data[1] = metadata->version_major;
    response->data[2] = metadata->version_minor;
    response->data[3] = metadata->version_patch;

    // CRC: 0x12345678 -> 0x12, 0x34, 0x56, 0x78
    response->data[4] = (metadata->crc >> 0) & 0xFF;
    response->data[5] = (metadata->crc >> 8) & 0xFF;
    response->data[6] = (metadata->crc >> 16) & 0xFF;
    response->data[7] = (metadata->crc >> 24) & 0xFF;

    // image body (firmware) size: 0x12345678 -> 0x12, 0x34, 0x56, 0x78
    response->data[8] = (metadata->size >> 0) & 0xFF;
    response->data[9] = (metadata->size >> 8) & 0xFF;
    response->data[10] = (metadata->size >> 16) & 0xFF;
    response->data[11] = (metadata->size >> 24) & 0xFF;
}
