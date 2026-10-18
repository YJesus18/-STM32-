#ifndef __APP_DISPLAY_H
#define __APP_DISPLAY_H

#include <stdint.h>

void AppDisplay_Init(void);
void AppDisplay_ShowBlueCount(uint8_t count);
void AppDisplay_ShowStop(void);
void AppDisplay_UpdateDistance(uint16_t distance_cm, uint8_t valid);
void AppDisplay_ShowTunnelEntered(void);
void AppDisplay_ShowTunnelResult(uint16_t count, uint32_t length_cm);

#endif
