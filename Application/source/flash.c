#include "flash.h"
#include "stm32f1xx_hal.h"

#define MCU_WORD_SIZE (4u)
#define FLASH_END (FLASH_BASE + FLASH_SIZE - 1)

static flash_status_t __flash_write_aligned(uint32_t *addr, uint32_t *data, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        if (HAL_FLASH_Program(
                FLASH_TYPEPROGRAM_WORD,
                (uint32_t) (addr + i),
                data[i]
            ) != HAL_OK) {
            return FLASH_ERROR;
        }
    }

    return FLASH_OK;
}

flash_status_t flash_write(uint32_t *addr, uint32_t *data, uint32_t length) {
    if (((uint32_t) addr % MCU_WORD_SIZE) > 0) {
        return FLASH_ALIGNMENT_ERROR;
    }

    if ((addr + length - 1) > (uint32_t *) FLASH_END) {
        return FLASH_ERROR;
    }

    __disable_irq();
    HAL_FLASH_Unlock();

    flash_status_t status = __flash_write_aligned(addr, data, length);    

    HAL_FLASH_Lock();
    __enable_irq();

    return status;
}

flash_status_t flash_erase(uint8_t page_start, uint8_t count) {
    if (page_start < 0 || page_start > 127) {
        return FLASH_ERROR;
    }

    if (page_start + count > 128) {
        return FLASH_ERROR;
    }

    FLASH_EraseInitTypeDef EraseInit;
    EraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInit.PageAddress = FLASH_PAGE_ADDR(page_start);
    EraseInit.NbPages = count;

    HAL_StatusTypeDef status = HAL_OK;
    uint32_t page_err = 0;

    __disable_irq();
    HAL_FLASH_Unlock();

    status = HAL_FLASHEx_Erase(&EraseInit, &page_err);
    if (status != HAL_OK) {
        return FLASH_ERROR;
    }

    HAL_FLASH_Lock();
    __enable_irq();

    return FLASH_OK;
}