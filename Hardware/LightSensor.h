#ifndef __LIGHT_SENSOR_H
#define __LIGHT_SENSOR_H

#include <stdint.h>

void LightSensor_Init(void);
uint8_t LightSensor_ReadDO(void);
uint16_t LightSensor_ReadAO(void);

#endif
