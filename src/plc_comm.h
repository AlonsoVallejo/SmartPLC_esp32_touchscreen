#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void plc_comm_init(void);
void plc_comm_task(void *pvParameters);

bool plc_get_input(int index);
void plc_set_output(int index, bool value);
bool plc_get_output(int index);

#ifdef __cplusplus
}
#endif