#include "stm32f10x.h"

/* TB6612 PWMA/PWMB: PB7=TIM4_CH2, PB8=TIM4_CH3. */
void PWM_Init(void)
{
	GPIO_InitTypeDef gpio;
	TIM_TimeBaseInitTypeDef timebase;
	TIM_OCInitTypeDef oc;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	gpio.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8;
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio);

	/* 72 MHz / 36 / 100 = 20 kHz PWM. */
	timebase.TIM_Period = 99;
	timebase.TIM_Prescaler = 35;
	timebase.TIM_ClockDivision = TIM_CKD_DIV1;
	timebase.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &timebase);

	TIM_OCStructInit(&oc);
	oc.TIM_OCMode = TIM_OCMode_PWM1;
	oc.TIM_OutputState = TIM_OutputState_Enable;
	oc.TIM_Pulse = 0;
	oc.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC2Init(TIM4, &oc);
	TIM_OC3Init(TIM4, &oc);
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM4, ENABLE);
	TIM_Cmd(TIM4, ENABLE);
}
