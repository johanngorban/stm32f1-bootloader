#pragma once

#include <stdint.h>
#include "stm32f1xx_hal.h"

#define FLASH_PAGE_ADDR(page) (FLASH_BASE + (page) * FLASH_PAGE_SIZE)

typedef enum {
    FLASH_OK = 0,
    FLASH_ERROR = 1,
    FLASH_ALIGNMENT_ERROR = 2,
} flash_status_t;

flash_status_t flash_write(uint32_t *addr, uint32_t *data, uint32_t length);

flash_status_t flash_erase(uint8_t page_start, uint8_t count);
