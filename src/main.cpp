#include <Arduino.h>

#include "gui.h"

void setup() {
    Serial.begin(9600);
    Serial.println("SmartPLC Touchscreen HMI starting...");

    gui_init();

    // Create FreeRTOS tasks
    xTaskCreatePinnedToCore(gui_task, "GUI", 8192, NULL, 1, NULL, 0);
}

void loop() {
    // The work is performed in FreeRTOS tasks.
    vTaskDelay(pdMS_TO_TICKS(1000));
}
