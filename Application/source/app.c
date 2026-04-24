#include "app.h"
#include "bcp.h"
#include "bcp_io.h"
#include "router.h"
#include "fwpio.h"
#include <stdbool.h>

extern UART_HandleTypeDef huart1;


void app_init() {
    bcp_uart_init(&huart1);
    fwp_init(&huart1);
}

void app_run() {
    bcp_request_t request;
    bcp_request_init(&request);

    bool activity_detected = false;

    while(1) {
        // if (!activity_detected && waited >= MAX_WAIT_TIME_MS) {
        //     jump_to_app();
        // }

        int8_t status = bcp_recv_request(&request);
        if (status == BCP_RECV_OK) {
            activity_detected = true;

            router_handle_request(&request);
            bcp_request_init(&request);
        }
    }
}