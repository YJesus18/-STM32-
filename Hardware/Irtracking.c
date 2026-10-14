#include "Irtracking.h"

/*
 * 物理顺序和布线:
 * 左侧  L1/X1->PA7 ... L8/X8->PA0  右侧
 * GPIO位是反转的，因此左侧为负，右侧为正
 */
static const int16_t SensorWeight[IRTRACKING_SENSOR_COUNT] =
{
	3500, 2500, 1500, 500, -500, -1500, -2500, -3500
};

// 初始化红外传感器GPIO
void Irtracking_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 |
		GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 |
		GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

// 读取红外传感器状态位图，1表示检测到黑线，0表示未检测到黑线
uint8_t Irtracking_GetPattern(void)
{
	uint8_t pattern;

	pattern = (uint8_t)(GPIO_ReadInputData(GPIOA) & 0x00FF);  // 读取GPIOA的值，只保留后八位

#if (IRTRACKING_BLACK_IS_HIGH == 0)
	pattern = (uint8_t)(~pattern);
#endif

	return pattern;
}

// 读取红外传感器数据
void Irtracking_Read(Irtracking_Data *data)
{
	uint8_t i;
	uint8_t pattern;
	int32_t weighted_sum;

	pattern = Irtracking_GetPattern();
	weighted_sum = 0;

	data->pattern = pattern; // 将状态位图存储到数据结构中
	data->black_count = 0;

	// 计算检测到黑线的传感器数量和加权和
	for (i = 0; i < IRTRACKING_SENSOR_COUNT; i++)
	{
		if ((pattern & (1U << i)) != 0)
		{
			weighted_sum += SensorWeight[i]; // 计算黑线的加权和
			data->black_count++;
		}
	}

	if (data->black_count > 0)
	{
		data->position = (int16_t)(weighted_sum / data->black_count); // 计算加权和的平均值，表示黑线位置
	}
	else
	{
		data->position = 0;
	}
}
