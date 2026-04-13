#pragma once

#include "fwp.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>

void fwp_init(UART_HandleTypeDef *huart);

fwp_status_t fwp_receive(uint8_t *dest, uint32_t *received_length);
