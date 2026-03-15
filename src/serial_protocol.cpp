#include "serial_protocol.h"
#include <Arduino.h>

/**
 * @brief Initialize serial communication for PLC protocol
 * @return void
 */
void serial_protocol_init(void) {
    PLC_SERIAL.begin(PLC_BAUD_RATE);
    PLC_SERIAL.println("Serial protocol initialized");
    delay(100); /* Allow time for serial to initialize */
}

/*
    * @brief Send output states to Arduino PLC
    * @param[in] outputs Output states packed as bits (0-5)
    * @return void
*/
void serial_protocol_send_outputs(uint8_t outputs) {
    char cmd[8];
    sprintf(cmd, "OUT:%02X\n", outputs);
    PLC_SERIAL.print(cmd);
}

/**
 * @brief Request current PLC state from Arduino
 * @return void
*/
void serial_protocol_request_state(void) {
    PLC_SERIAL.println("REQ");
}

/**
 * @brief Parse Arduino PLC state response
 * @param[in] buffer Response string
 * @param[out] inputs Parsed input states (packed bits)
 * @param[out] outputs Parsed output states (packed bits)
 * @return true if parsing successful, false otherwise
 */
bool serial_protocol_parse_state_response(const char *buffer, uint8_t *inputs, uint8_t *outputs) {

    if (sscanf(buffer, "STATE:%hhx:%hhx", inputs, outputs) == 2) {
        return true;
    }
    return false;
}