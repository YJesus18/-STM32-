#include "Irtracking.h"

/*
 * 物理顺序和布线:
 * 左侧  L1/X1->PA7 ... L8/X8->PA0  右侧
 * GPIO位是反转的，因此左侧为负，右侧为正
 */
#define TRACK_LEFT_OUTER_PIN   GPIO_Pin_5
#define TRACK_LEFT_INNER_PIN   GPIO_Pin_4
#define TRACK_RIGHT_INNER_PIN  GPIO_Pin_3
#define TRACK_RIGHT_OUTER_PIN  GPIO_Pin_2
#define TRACK_FOUR_PINS        (TRACK_LEFT_OUTER_PIN | TRACK_LEFT_INNER_PIN | \
	TRACK_RIGHT_INNER_PIN | TRACK_RIGHT_OUTER_PIN)

// 初始化红外传感器GPIO
void Irtracking_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = TRACK_FOUR_PINS;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

// 读取红外传感器状态位图，1表示检测到黑线，0表示未检测到黑线
uint8_t Irtracking_GetPattern(void)
{
	uint8_t pattern;

	pattern = (uint8_t)(GPIO_ReadInputData(GPIOA) & TRACK_FOUR_PINS);

#if (IRTRACKING_BLACK_IS_HIGH == 0)
	pattern = (uint8_t)((~pattern) & TRACK_FOUR_PINS);
#endif

	return pattern;
}

// 读取红外传感器数据
void Irtracking_Read(Irtracking_Data *data)
{
	uint8_t pattern;

	pattern = Irtracking_GetPattern();
	data->pattern = pattern;
	data->black_count = 0U;
	data->position = 0;

	if ((pattern & TRACK_LEFT_OUTER_PIN) != 0U)
	{
		data->black_count++;
		data->position -= 1500;
	}
	if ((pattern & TRACK_LEFT_INNER_PIN) != 0U)
	{
		data->black_count++;
		data->position -= 500;
	}
	if ((pattern & TRACK_RIGHT_INNER_PIN) != 0U)
	{
		data->black_count++;
		data->position += 500;
	}
	if ((pattern & TRACK_RIGHT_OUTER_PIN) != 0U)
	{
		data->black_count++;
		data->position += 1500;
	}

	if (data->black_count != 0U)
	{
		data->position /= data->black_count;
	}
}
