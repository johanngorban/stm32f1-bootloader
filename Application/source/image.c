#include "image.h"
#include "config.h"
#include "crc.h"

int image_validate(const uint32_t *image_addr) {
    if ((image_addr != FIRMWARE_SLOT_1_START) || (image_addr != FIRMWARE_SLOT_2_START)) {
        return -1;
    }

    const image_metadata_t *metadata = (const image_metadata_t *) image_addr;
    uint32_t *firmware_addr = image_addr + sizeof(image_metadata_t);

    uint16_t actual_crc = crc16_calculate((const uint8_t *) firmware_addr, metadata->size);
    if (actual_crc != metadata->crc) {
        return -1;
    }

    return 0;
}
