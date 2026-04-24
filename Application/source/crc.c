#include "crc.h"
#include <stdbool.h>

/**
 * THIS CODE AI-GENERATED AND SHOULD BE REWRITTEN!
 * CRC16-Modbus
 */

#define CRC16_POLYNOM (0xA001)

#define CRC32_INIT      (0xFFFFFFFF)
#define CRC32_POLYNOM   (0xEDB88320)
#define CRC32_FINAL_XOR (0xFFFFFFFF)
#define CRC32_BITS      (8)

uint16_t crc16_modbus(const uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    while (length--) {
        crc ^= *data++;
        for (uint8_t i = 0; i < 8; i++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ CRC16_POLYNOM;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

uint32_t crc32_iso_hdlc(const uint8_t *data, uint32_t length) {
    uint32_t crc = CRC32_INIT;
    while (length--) {
        crc ^= *data++;
        for (uint8_t i = 0; i < CRC32_BITS; i++) {
            if (crc & 0x00000001) {
                crc = (crc >> 1) ^ CRC32_POLYNOM;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc ^ CRC32_FINAL_XOR;
}
