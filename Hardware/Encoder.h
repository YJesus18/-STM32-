#ifndef __ENCODER_H
#define __ENCODER_H

#include <stdint.h>

void Encoder_Init(void);
int32_t Encoder_GetLeftCount(void);
int32_t Encoder_GetRightCount(void);
void Encoder_ResetCounts(void);

#endif
