#pragma once

#include <stdint.h>

#define CRC16_POLYNOM (0xA001)

uint16_t crc16_modbus(const uint8_t *data, uint16_t length);

uint32_t crc32_iso_hdlc(const uint8_t *data, uint32_t length);
