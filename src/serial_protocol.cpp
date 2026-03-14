#include "serial_protocol.h"
#include <Arduino.h>

void serial_protocol_init(void) {
    PLC_SERIAL.begin(PLC_BAUD_RATE);
    PLC_SERIAL.println("Serial protocol initialized");
    delay(100);  // Allow time for serial to initialize
}

void serial_protocol_send_outputs(uint8_t outputs) {
    // Send command: OUT:xx where xx is 2 hex digits
    char cmd[8];
    sprintf(cmd, "OUT:%02X\n", outputs);
    PLC_SERIAL.print(cmd);
}

void serial_protocol_request_state(void) {
    // Send command to request state
    PLC_SERIAL.println("REQ");
}

bool serial_protocol_parse_state_response(const char *buffer, uint8_t *inputs, uint8_t *outputs) {
    // Expected response: STATE:in:xx where in and xx are hex
    if (sscanf(buffer, "STATE:%hhx:%hhx", inputs, outputs) == 2) {
        return true;
    }
    return false;
}