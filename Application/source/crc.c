#include "crc.h"
#include <stdbool.h>

/**
 * THIS CODE AI-GENERATED AND SHOULD BE REWRITTEN!
 * CRC16-Modbus
 */

uint16_t crc16_modbus(const uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    while (length--) {
        crc ^= *data++;
        for (uint8_t i = 0; i < 8; i++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ CRC16_POLYNOM;
            }
            else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

uint32_t crc32_iso_hdlc(const uint8_t *data, uint32_t length) {

}
