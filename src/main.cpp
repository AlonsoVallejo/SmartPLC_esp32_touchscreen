#include <Arduino.h>

#include "gui.h"
#include "plc_comm.h"

void setup() {
    /* @brief Initialize PLC communication and GUI, then start FreeRTOS tasks */
    plc_comm_init();
    gui_init();

    /* @brief Start PLC and GUI tasks */
    xTaskCreatePinnedToCore(plc_comm_task, "PLC_Comm", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(gui_task, "GUI", 8192, NULL, 1, NULL, 0);
}

void loop() {
    /* @brief Idle loop, all work is done in tasks */
    vTaskDelay(pdMS_TO_TICKS(1000));
}
