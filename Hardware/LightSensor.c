#include "stm32f10x.h"
#include "LightSensor.h"

/* HU591-style light module: DO=PB5, AO=PB1 (ADC1 channel 9). */
#define LIGHT_DO_PIN       GPIO_Pin_5
#define LIGHT_AO_PIN       GPIO_Pin_1
#define LIGHT_ADC_TIMEOUT  100000U

void LightSensor_Init(void)
{
	GPIO_InitTypeDef gpio;
	ADC_InitTypeDef adc;
	uint32_t timeout;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC1, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);

	gpio.GPIO_Pin = LIGHT_DO_PIN;
	gpio.GPIO_Mode = GPIO_Mode_IPU;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &gpio);

	gpio.GPIO_Pin = LIGHT_AO_PIN;
	gpio.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOB, &gpio);

	ADC_DeInit(ADC1);
	adc.ADC_Mode = ADC_Mode_Independent;
	adc.ADC_ScanConvMode = DISABLE;
	adc.ADC_ContinuousConvMode = DISABLE;
	adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	adc.ADC_DataAlign = ADC_DataAlign_Right;
	adc.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &adc);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_239Cycles5);
	ADC_Cmd(ADC1, ENABLE);

	ADC_ResetCalibration(ADC1);
	timeout = LIGHT_ADC_TIMEOUT;
	while (ADC_GetResetCalibrationStatus(ADC1) == SET && timeout > 0U)
	{
		timeout--;
	}

	ADC_StartCalibration(ADC1);
	timeout = LIGHT_ADC_TIMEOUT;
	while (ADC_GetCalibrationStatus(ADC1) == SET && timeout > 0U)
	{
		timeout--;
	}
}

uint8_t LightSensor_ReadDO(void)
{
	return (uint8_t)GPIO_ReadInputDataBit(GPIOB, LIGHT_DO_PIN);
}

uint16_t LightSensor_ReadAO(void)
{
	uint32_t timeout;

	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	timeout = LIGHT_ADC_TIMEOUT;
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET && timeout > 0U)
	{
		timeout--;
	}

	if (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
	{
		return 0;
	}

	return ADC_GetConversionValue(ADC1);
}
