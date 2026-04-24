#pragma once

#include <stdint.h>
#include <assert.h>

#define IMAGE_METADATA_SIZE (32u)
#define IMAGE_MAGIC_NUMBER  (0xAAAAAAAAu)

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t size;
    uint32_t crc;
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_patch;
    uint8_t reserved[IMAGE_METADATA_SIZE - 15]; // For padding;
} image_metadata_t;

static_assert(sizeof(image_metadata_t) == IMAGE_METADATA_SIZE, "image_metadata_t must be exactly 32 bytes length");

// Return 0 on success, -1 otherwise
int image_validate(const uint32_t *image_addr);
