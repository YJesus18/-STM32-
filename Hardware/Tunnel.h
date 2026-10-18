#ifndef __TUNNEL_H
#define __TUNNEL_H

#include <stdint.h>

void Tunnel_Init(void);
void Tunnel_Update(void);
uint8_t Tunnel_IsInside(void);
uint8_t Tunnel_HasEntered(void);
uint8_t Tunnel_HasNewResult(void);
uint16_t Tunnel_GetCount(void);
uint32_t Tunnel_GetLengthCm(void);
uint32_t Tunnel_GetLengthPulses(void);
uint16_t Tunnel_GetDistanceCm(void);
uint8_t Tunnel_IsDistanceValid(void);

#endif
