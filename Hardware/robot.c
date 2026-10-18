#include "stm32f10x.h"
#include "PWM.h"
#include "Delay.h"
#include "robot.h"

#define TB6612_STBY_PIN GPIO_Pin_6
#define TB6612_AIN1_PIN GPIO_Pin_0
#define TB6612_AIN2_PIN GPIO_Pin_9
#define TB6612_BIN1_PIN GPIO_Pin_11
#define TB6612_BIN2_PIN GPIO_Pin_12

static uint8_t LimitMotorSpeed(uint8_t speed)
{
	return (speed > 100U) ? 100U : speed;
}

void robot_Init(void)
{
	GPIO_InitTypeDef gpio;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_ResetBits(GPIOA, TB6612_AIN2_PIN | TB6612_BIN1_PIN |
		TB6612_BIN2_PIN);
	GPIO_ResetBits(GPIOB, TB6612_STBY_PIN | TB6612_AIN1_PIN);

	gpio.GPIO_Pin = TB6612_AIN2_PIN | TB6612_BIN1_PIN | TB6612_BIN2_PIN;
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio);

	gpio.GPIO_Pin = TB6612_STBY_PIN | TB6612_AIN1_PIN;
	GPIO_Init(GPIOB, &gpio);

	/* Reverse both motor directions: AIN2/BIN2 high, AIN1/BIN1 low. */
	GPIO_SetBits(GPIOA, TB6612_AIN2_PIN | TB6612_BIN2_PIN);
	PWM_Init();
	robot_speed(0, 0);
	GPIO_SetBits(GPIOB, TB6612_STBY_PIN);
}

/* Speed is a duty percentage from 0 to 100. */
void robot_speed(uint8_t left_speed, uint8_t right_speed)
{
	left_speed = LimitMotorSpeed(left_speed);
	right_speed = LimitMotorSpeed(right_speed);
	TIM_SetCompare2(TIM4, left_speed);
	TIM_SetCompare3(TIM4, right_speed);
}

void makerobo_run(uint8_t speed, uint16_t time)
{
	robot_speed(speed, speed);
	Delay_ms(time);
}

void makerobo_brake(uint16_t time)
{
	robot_speed(0, 0);
	Delay_ms(time);
}

/* With forward-only wiring these are gentle differential turns, not pivots. */
void makerobo_Left(int8_t speed, uint16_t time)
{
	uint8_t duty = (speed < 0) ? 0U : (uint8_t)speed;
	robot_speed(0, duty);
	Delay_ms(time);
}

void makerobo_Right(int8_t speed, uint16_t time)
{
	uint8_t duty = (speed < 0) ? 0U : (uint8_t)speed;
	robot_speed(duty, 0);
	Delay_ms(time);
}
