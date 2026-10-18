#ifndef __VISION_CONTROL_H
#define __VISION_CONTROL_H

#include <stdint.h>

void VisionControl_Init(void);
void VisionControl_Update(void);
uint8_t VisionControl_HasNewBlueCount(void);
uint8_t VisionControl_GetBlueCount(void);
uint8_t VisionControl_HasRedStopRequest(void);

#endif
