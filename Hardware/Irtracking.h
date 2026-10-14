#ifndef __IRTRACKING_H
#define __IRTRACKING_H

#include "stm32f10x.h"

#define IRTRACKING_SENSOR_COUNT  8

/*
 * S1 is the leftmost sensor and S8 is the rightmost sensor.
 * Wiring: S8->PA0, S7->PA1, ..., S1->PA7.
 * This module is configured for low-level output on a black line.
 */
#define IRTRACKING_BLACK_IS_HIGH  0

// 红外传感器读取结果
typedef struct
{
	uint8_t pattern; // 传感器状态位图，低位对应右边传感器，高位对应左边传感器
	uint8_t black_count; // 检测到黑线的传感器数量
	int16_t position; // 黑线位置，范围[-7, 7]，左边为负，右边为正
} Irtracking_Data;

void Irtracking_Init(void);
uint8_t Irtracking_GetPattern(void);
void Irtracking_Read(Irtracking_Data *data);

#endif
