#include "plc_comm.h"
#include "serial_protocol.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

/* PLC input states */
static bool s_inputs[6] = {false, false, false, false, false, false};
/* PLC output states */
static bool s_outputs[6] = {false, false, false, false, false, false};
/* Mutex for thread safety */
static SemaphoreHandle_t s_mutex = NULL;

/**
 * @brief Initialize PLC communication task and serial protocol
 * @return void
 */
void plc_comm_init(void) {
    if (s_mutex == NULL) {
        s_mutex = xSemaphoreCreateMutex();
    }
    serial_protocol_init();
}

/**
 * @brief Get input state
 * @param[in] index Input index (0-5)
 * @return true if input is ON, false otherwise
 */
bool plc_get_input(int index) {

    bool value = false;
    if (index >= 0 && index < 6 && s_mutex) {
        if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(10))) {
            value = s_inputs[index];
            xSemaphoreGive(s_mutex);
        }
    }
    return value;
}

/**
 * @brief Get output state
 * @param[in] index Output index (0-5)
 * @return true if output is ON, false otherwise
 */
bool plc_get_output(int index) {
    bool value = false;
    if (index >= 0 && index < 6 && s_mutex) {
        if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(10))) {
            value = s_outputs[index];
            xSemaphoreGive(s_mutex);
        }
    }
    return value;
}

/**
 * @brief Set output state and send to Arduino
 * @param[in] index Output index (0-5)
 * @param[in] value Output state (true=ON, false=OFF)
 * @return void
 */
void plc_set_output(int index, bool value) {

    if (index >= 0 && index < 6 && s_mutex) {
        if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(10))) {
            s_outputs[index] = value;
            /* Compute outputs byte */
            uint8_t outputs_byte = 0;
            for (int i = 0; i < 6; i++) {
                if (s_outputs[i]) {
                    outputs_byte |= (1 << i);
                }
            }
            xSemaphoreGive(s_mutex);
            /* Send the command to Arduino */
            serial_protocol_send_outputs(outputs_byte);
        }
    }
}

/**
 * @brief PLC communication task: poll Arduino for state and update local arrays
 * @param[in] pvParameters FreeRTOS task parameter (unused)
 * @return void
 */
void plc_comm_task(void *pvParameters) {
    (void)pvParameters;

    static char buffer[64];
    static int buffer_index = 0;

    while (true) {
        /* Request state from Arduino every 1 second */
        serial_protocol_request_state();
        vTaskDelay(pdMS_TO_TICKS(1000));

        /* Check for incoming data */
        while (PLC_SERIAL.available()) {
            char c = PLC_SERIAL.read();
            PLC_SERIAL.println(c);
            if (c == '\n' || c == '\r') {
                if (buffer_index > 0) {
                    buffer[buffer_index] = '\0';
                    /* Parse the response */
                    uint8_t inputs_byte, outputs_byte;
                    if (serial_protocol_parse_state_response(buffer, &inputs_byte, &outputs_byte)) {
                        /* Update inputs and outputs */
                        if (s_mutex && xSemaphoreTake(s_mutex, pdMS_TO_TICKS(10))) {
                            for (int i = 0; i < 6; i++) {
                                s_inputs[i] = (inputs_byte & (1 << i)) != 0;
                                s_outputs[i] = (outputs_byte & (1 << i)) != 0;
                            }
                            xSemaphoreGive(s_mutex);
                        }
                    }
                    buffer_index = 0;
                }
            } else if (buffer_index < sizeof(buffer) - 1) {
                buffer[buffer_index++] = c;
            }
        }
    }
}