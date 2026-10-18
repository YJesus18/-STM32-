#include "stm32f10x.h"
#include "Delay.h"
#include "Ultrasonic.h"

/* HC-SR04 wiring: PB9=TRIG, PA8=ECHO (ECHO must be divided to 3.3 V). */
#define ULTRASONIC_TRIG_PIN          GPIO_Pin_9
#define ULTRASONIC_ECHO_PIN          GPIO_Pin_8
#define ULTRASONIC_PERIOD_MS         60U
#define ULTRASONIC_TIMEOUT_MS        30U
#define ULTRASONIC_MIN_ECHO_US       100U
#define ULTRASONIC_MAX_ECHO_US       25000U

static volatile uint8_t EchoWaiting;
static volatile uint8_t EchoRisingSeen;
static volatile uint8_t MeasurementReady;
static volatile uint8_t MeasurementValid;
static volatile uint16_t DistanceCm;
static uint16_t TriggerElapsedMs;
static uint16_t EchoElapsedMs;

void Ultrasonic_Init(void)
{
	GPIO_InitTypeDef gpio;
	TIM_TimeBaseInitTypeDef timer;
	EXTI_InitTypeDef exti;
	NVIC_InitTypeDef nvic;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
		RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO |
		RCC_APB2Periph_TIM1, ENABLE);

	gpio.GPIO_Pin = ULTRASONIC_TRIG_PIN;
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio);
	GPIO_ResetBits(GPIOB, ULTRASONIC_TRIG_PIN);

	gpio.GPIO_Pin = ULTRASONIC_ECHO_PIN;
	gpio.GPIO_Mode = GPIO_Mode_IPD;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &gpio);

	/* TIM1 runs freely at 1 MHz, so one timer count equals one microsecond. */
	timer.TIM_Period = 65535U;
	timer.TIM_Prescaler = 71U;
	timer.TIM_ClockDivision = TIM_CKD_DIV1;
	timer.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM1, &timer);
	TIM_SetCounter(TIM1, 0U);
	TIM_Cmd(TIM1, ENABLE);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8);
	EXTI_ClearITPendingBit(EXTI_Line8);
	exti.EXTI_Line = EXTI_Line8;
	exti.EXTI_Mode = EXTI_Mode_Interrupt;
	exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	exti.EXTI_LineCmd = ENABLE;
	EXTI_Init(&exti);

	nvic.NVIC_IRQChannel = EXTI9_5_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 2U;
	nvic.NVIC_IRQChannelSubPriority = 1U;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	EchoWaiting = 0U;
	EchoRisingSeen = 0U;
	MeasurementReady = 0U;
	MeasurementValid = 0U;
	DistanceCm = 0U;
	TriggerElapsedMs = 0U;
	EchoElapsedMs = 0U;
}

void Ultrasonic_Update(void)
{
	if (EchoWaiting != 0U)
	{
		if (EchoElapsedMs < ULTRASONIC_TIMEOUT_MS)
		{
			EchoElapsedMs++;
		}
		else
		{
			EchoWaiting = 0U;
			EchoRisingSeen = 0U;
			MeasurementValid = 0U;
			MeasurementReady = 1U;
		}
	}

	if (TriggerElapsedMs < ULTRASONIC_PERIOD_MS)
	{
		TriggerElapsedMs++;
	}

	if ((TriggerElapsedMs >= ULTRASONIC_PERIOD_MS) &&
		(EchoWaiting == 0U))
	{
		TriggerElapsedMs = 0U;
		EchoElapsedMs = 0U;
		EchoRisingSeen = 0U;
		EchoWaiting = 1U;

		GPIO_SetBits(GPIOB, ULTRASONIC_TRIG_PIN);
		Delay_us(12U);
		GPIO_ResetBits(GPIOB, ULTRASONIC_TRIG_PIN);
	}
}

uint8_t Ultrasonic_HasNewMeasurement(void)
{
	uint8_t ready = MeasurementReady;
	MeasurementReady = 0U;
	return ready;
}

uint8_t Ultrasonic_IsValid(void)
{
	return MeasurementValid;
}

uint16_t Ultrasonic_GetDistanceCm(void)
{
	return DistanceCm;
}

void EXTI9_5_IRQHandler(void)
{
	uint16_t echo_us;

	if (EXTI_GetITStatus(EXTI_Line8) == RESET)
	{
		return;
	}
	EXTI_ClearITPendingBit(EXTI_Line8);

	if ((GPIO_ReadInputDataBit(GPIOA, ULTRASONIC_ECHO_PIN) != 0U) &&
		(EchoWaiting != 0U))
	{
		TIM_SetCounter(TIM1, 0U);
		EchoRisingSeen = 1U;
	}
	else if ((EchoWaiting != 0U) && (EchoRisingSeen != 0U))
	{
		echo_us = (uint16_t)TIM_GetCounter(TIM1);
		EchoWaiting = 0U;
		EchoRisingSeen = 0U;
		MeasurementReady = 1U;

		if ((echo_us >= ULTRASONIC_MIN_ECHO_US) &&
			(echo_us <= ULTRASONIC_MAX_ECHO_US))
		{
			DistanceCm = (uint16_t)((echo_us + 29U) / 58U);
			MeasurementValid = 1U;
		}
		else
		{
			MeasurementValid = 0U;
		}
	}
}
