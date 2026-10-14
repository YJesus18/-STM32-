#ifndef __ROBOT_H
#define __ROBOT_H

#include <stdint.h>

void robot_Init(void);
void robot_speed(uint8_t left_speed, uint8_t right_speed);
void makerobo_run(uint8_t speed, uint16_t time);
void makerobo_brake(uint16_t time);
void makerobo_Left(int8_t speed, uint16_t time);
void makerobo_Right(int8_t speed, uint16_t time);

#endif
