#include "stm32f10x.h"                  // Device header
#include "Serial.h"
#include <stdio.h>

#include <stdarg.h>

#define SERIAL_RX_BUFFER_SIZE 32U

static volatile uint8_t Serial_RxBuffer[SERIAL_RX_BUFFER_SIZE];
static volatile uint8_t Serial_RxHead;
static volatile uint8_t Serial_RxTail;

uint8_t Serial_ReadByte(uint8_t *data);

void Serial_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStruture;
	USART_InitStruture.USART_BaudRate = 115200;
	USART_InitStruture.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruture.USART_Mode = USART_Mode_Rx;
	USART_InitStruture.USART_Parity = USART_Parity_No;
	USART_InitStruture.USART_StopBits = USART_StopBits_1;
	USART_InitStruture.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1,&USART_InitStruture);
	
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	Serial_RxHead = 0U;
	Serial_RxTail = 0U;
	USART_Cmd(USART1,ENABLE);
}

void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART1,Byte);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
	
}

void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for(i = 0;i < Length; i++)
	{
		Serial_SendByte(Array[i]);
	}
}

void Serial_SendString(char *String)
{
	uint8_t i;
	for (i = 0;String[i] != '\0';i++)
	{
		Serial_SendByte(String[i]);
	}
}

uint32_t Serial_Pow(uint32_t X,uint32_t Y)
{
	uint32_t Result = 1;
	while(Y --)
	{
		Result *=X;
	}
	return Result;
}

void Serial_SendNumber(uint32_t Number,uint8_t Length)
{
	uint8_t i;
	for(i = 0; i < Length; i++)
	{
		Serial_SendByte(Number / Serial_Pow(10,Length - i - 1) % 10 + '0');
	}
}

int fputc(int ch,FILE *f)	
{
	Serial_SendByte(ch);
	return ch;
}


void Serial_Printf(char *format, ...)
{
	char String[100];
	va_list arg;
	va_start(arg,format);
	vsprintf(String,format,arg);
	va_end(arg);
	Serial_SendString(String);
	
}

uint8_t Serial_GetRxFlag(void)
{
	return (Serial_RxHead != Serial_RxTail) ? 1U : 0U;
}

uint8_t Serial_GetRxData(void)
{
	uint8_t data = 0U;
	(void)Serial_ReadByte(&data);
	return data;
}

uint8_t Serial_ReadByte(uint8_t *data)
{
	if ((data == 0) || (Serial_RxHead == Serial_RxTail))
	{
		return 0U;
	}

	*data = Serial_RxBuffer[Serial_RxTail];
	Serial_RxTail = (uint8_t)((Serial_RxTail + 1U) % SERIAL_RX_BUFFER_SIZE);
	return 1U;
}

void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		uint8_t data;
		uint8_t next;

		data = (uint8_t)USART_ReceiveData(USART1);
		next = (uint8_t)((Serial_RxHead + 1U) % SERIAL_RX_BUFFER_SIZE);
		if (next != Serial_RxTail)
		{
			Serial_RxBuffer[Serial_RxHead] = data;
			Serial_RxHead = next;
		}
	}
}
