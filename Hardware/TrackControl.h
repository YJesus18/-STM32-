#ifndef __TRACK_CONTROL_H
#define __TRACK_CONTROL_H

#include <stdint.h>

void TrackControl_Init(void);
void TrackControl_Update(void);
void TrackControl_Stop(void);
uint8_t TrackControl_GetBlackCount(void);

#endif
