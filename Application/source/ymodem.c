#include "ymodem.h"
#include <stddef.h>
#include <stdbool.h>

static ymodem_config_t config = {};
static bool ymodem_inited = false;

uint8_t ymodem_init(ymodem_config_t *cfg) {
    if (cfg == NULL || ymodem_inited == true) {
        return 1;
    }

    if (cfg->ctx == NULL) {
        return 1;
    }

    if (cfg->read == NULL) {
        return 1;
    }

    if (cfg->write == NULL) {
        return 1;
    }

    config = *cfg;
    ymodem_inited = true;
    return 0;
}

ymodem_status_t ymodem_receive(uint8_t *data, uint32_t *length) {
    if (ymodem_inited == false) {
        return YMODEM_MODULE_NOT_INITED;
    }

    ymodem_io_func read = config.read;
    
    uint8_t byte = 0;
    read(&byte, 1, config.ctx);

    uint32_t 
}

ymodem_status_t ymodem_transmit(uint8_t *data, uint32_t *length) {
    if (ymodem_inited == false) {
        return YMODEM_MODULE_NOT_INITED;
    }

    ymodem_io_func write = config.write;
}