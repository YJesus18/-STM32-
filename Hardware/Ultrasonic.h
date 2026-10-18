#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H

#include <stdint.h>

void Ultrasonic_Init(void);
void Ultrasonic_Update(void);
uint8_t Ultrasonic_HasNewMeasurement(void);
uint8_t Ultrasonic_IsValid(void);
uint16_t Ultrasonic_GetDistanceCm(void);

#endif
