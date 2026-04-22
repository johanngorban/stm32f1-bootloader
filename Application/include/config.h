#pragma once

#define MAX_WAIT_TIME_MS    (10000)

/**
 * Bootloader version
 */
#define BOOTLOADER_MAJOR_VERSION (0)
#define BOOTLOADER_MINOR_VERSION (0)
#define BOOTLOADER_PATCH_VERSION (0)
#define MAX_WAIT_TIME_MS (10000)

/**
 * Memory map
 */
#define BOOTLOADER_START ((volatile uint8_t *) 0x08000000)
#define BOOTLOADER_SIZE  (16 * 1024) // 16Kb

#define FLASH_SIZE ((*(volatile uint16_t *) 0x1FFFF7E0) * 1024)

#define FIRMWARE_SLOT_SIZE   (48 * 1024)
#define FIRMWARE_SLOT_1_START (BOOTLOADER_START + BOOTLOADER_SIZE)
#define FIRMWARE_SLOT_2_START (FIRMWARE_SLOT_1_START + FIRMWARE_SLOT_SIZE)
