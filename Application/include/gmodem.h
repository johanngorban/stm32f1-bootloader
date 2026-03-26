#pragma once

#include "stm32f1xx_hal.h"

#include <stdint.h>

#define GMODEM_SOH  (0x01)
#define GMODEM_EOT  (0x04)
#define GMODEM_ACK  (0x06)
#define GMODEM_NAK  (0x15)
#define GMODEM_HEADER_SIZE (4)
#define GMODEM_FOOTER_SIZE (2)
#define GMODEM_DATA_SIZE   (1024)
#define GMODEM_PACKET_SIZE (GMODEM_HEADER_SIZE + GMODEM_DATA_SIZE + GMODEM_FOOTER_SIZE)

void gmodem_set_uart(UART_HandleTypeDef *huart);

uint32_t gmodem_receive_block(uint8_t *data);