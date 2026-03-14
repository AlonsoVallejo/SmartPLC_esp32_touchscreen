#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Serial communication settings
#define PLC_SERIAL Serial  // Use Serial2 for communication with Arduino
#define PLC_BAUD_RATE 9600

// Serial protocol functions
void serial_protocol_init(void);
void serial_protocol_send_outputs(uint8_t outputs);
void serial_protocol_request_state(void);
bool serial_protocol_parse_state_response(const char *buffer, uint8_t *inputs, uint8_t *outputs);

#ifdef __cplusplus
}
#endif