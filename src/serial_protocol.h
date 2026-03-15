#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PLC_SERIAL_RX_PIN 16
#define PLC_SERIAL_TX_PIN 17
/* Serial communication settings */
#define PLC_SERIAL Serial  // Use Serial for communication with Arduino
#define PLC_BAUD_RATE 9600

/**
 * @brief Initialize serial communication for PLC protocol
 * @return void
 */
void serial_protocol_init(void);

/**
 * @brief Send output states to Arduino PLC
 * @param[in] outputs Output states packed as bits (0-5)
 * @return void
 */
void serial_protocol_send_outputs(uint8_t outputs);

/**
 * @brief Request current PLC state from Arduino
 * @return void
 */
void serial_protocol_request_state(void);

/**
 * @brief Parse Arduino PLC state response
 * @param[in] buffer Response string
 * @param[out] inputs Parsed input states (packed bits)
 * @param[out] outputs Parsed output states (packed bits)
 * @return true if parsing successful, false otherwise
 */
bool serial_protocol_parse_state_response(const char *buffer, uint8_t *inputs, uint8_t *outputs);

#ifdef __cplusplus
}
#endif