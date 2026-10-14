#include "stm32f10x.h"
#include "Encoder.h"

/* Connector order: PB12=E2B(left), PB13=E2A(left), PB14=E1B(right), PB15=E1A(right). */
#define LEFT_A_PIN  GPIO_Pin_13
#define LEFT_B_PIN  GPIO_Pin_12
#define RIGHT_A_PIN GPIO_Pin_15
#define RIGHT_B_PIN GPIO_Pin_14
#define ENCODER_PINS (LEFT_A_PIN | LEFT_B_PIN | RIGHT_A_PIN | RIGHT_B_PIN)

static volatile int32_t left_count;
static volatile int32_t right_count;
static uint8_t previous_left_state;
static uint8_t previous_right_state;

/* Positive sequence: 00 -> 01 -> 11 -> 10 -> 00. */
static const int8_t QuadratureStep[16] =
{
	0, 1, -1, 0,
	-1, 0, 0, 1,
	1, 0, 0, -1,
	0, -1, 1, 0
};

static uint8_t ReadLeftState(uint16_t pins)
{
	return (uint8_t)(((pins & LEFT_A_PIN) ? 1U : 0U) |
		((pins & LEFT_B_PIN) ? 2U : 0U));
}

static uint8_t ReadRightState(uint16_t pins)
{
	return (uint8_t)(((pins & RIGHT_A_PIN) ? 1U : 0U) |
		((pins & RIGHT_B_PIN) ? 2U : 0U));
}

void Encoder_Init(void)
{
	GPIO_InitTypeDef gpio;
	EXTI_InitTypeDef exti;
	NVIC_InitTypeDef nvic;
	uint16_t pins;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

	gpio.GPIO_Pin = ENCODER_PINS;
	gpio.GPIO_Mode = GPIO_Mode_IPU;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &gpio);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource13);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource15);

	EXTI_ClearITPendingBit(EXTI_Line12 | EXTI_Line13 | EXTI_Line14 | EXTI_Line15);
	exti.EXTI_Line = EXTI_Line12 | EXTI_Line13 | EXTI_Line14 | EXTI_Line15;
	exti.EXTI_Mode = EXTI_Mode_Interrupt;
	exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	exti.EXTI_LineCmd = ENABLE;
	EXTI_Init(&exti);

	nvic.NVIC_IRQChannel = EXTI15_10_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 2;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	pins = GPIO_ReadInputData(GPIOB);
	previous_left_state = ReadLeftState(pins);
	previous_right_state = ReadRightState(pins);
	left_count = 0;
	right_count = 0;
}

void EXTI15_10_IRQHandler(void)
{
	uint16_t pins;
	uint8_t state;
	uint32_t pending;

	pending = EXTI->PR & (EXTI_Line12 | EXTI_Line13 | EXTI_Line14 | EXTI_Line15);
	if (pending == 0U)
	{
		return;
	}
	EXTI_ClearITPendingBit(pending);

	/* Read both channels together so a transition is decoded as one quadrature state. */
	pins = GPIO_ReadInputData(GPIOB);
	state = ReadLeftState(pins);
	left_count += QuadratureStep[(previous_left_state << 2) | state];
	previous_left_state = state;

	state = ReadRightState(pins);
	right_count += QuadratureStep[(previous_right_state << 2) | state];
	previous_right_state = state;
}

int32_t Encoder_GetLeftCount(void)
{
	return left_count;
}

int32_t Encoder_GetRightCount(void)
{
	return right_count;
}

void Encoder_ResetCounts(void)
{
	__disable_irq();
	left_count = 0;
	right_count = 0;
	__enable_irq();
}
