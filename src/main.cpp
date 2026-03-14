#include <Arduino.h>

#include "gui.h"
#include "plc_comm.h"

void setup() {
    plc_comm_init();
    gui_init();

    xTaskCreatePinnedToCore(plc_comm_task, "PLC_Comm", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(gui_task, "GUI", 8192, NULL, 1, NULL, 0);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
