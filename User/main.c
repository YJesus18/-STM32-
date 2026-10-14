#include "stm32f10x.h"
#include "Delay.h"
#include "robot.h"
#include "Irtracking.h"
#include "Encoder.h"
#include "OLED.h"
#include "LightSensor.h"

#define TRACK_BASE_SPEED       26 // 正常循迹基础PWM占空比
#define TRACK_CROSS_SPEED      25 // 十字路口速度
#define TRACK_SEARCH_SPEED     37 // 丢线搜索速度
#define TRACK_LOST_STOP_COUNT  5000 // 连续丢线约 100 次后停车（主循环周期约 5 ms）
#define TRACK_PWM_MULTIPLIER   1 // PWM倍数
#define MOTOR_LEFT_OFFSET      2 // 新增：电机差速补偿（左轮基准偏移） /小车固定往左偏就加正值，固定往右偏就加负值*/

static int16_t LastPosition = 0; // 上一次黑线的位置
static uint16_t LostCount = 0; // 连续丢线次数
static volatile uint8_t TrackBlackCount = 0;
static uint16_t TunnelCount = 0; // 隧道识别逻辑完成前保持为0
// 限制速度在 0 到 100 之间
static int16_t LimitSpeed(int16_t speed)
{
	if (speed > 100)
	{
		return 100;
	}
	if (speed < 0)
	{
		return 0;
	}
	return speed;
}

// 根据黑线位置计算纠偏值
static int16_t GetTrackCorrection(int16_t position)
{
	if ((position > -160) && (position < 160))
	{
		return 0;
	}

if (position < 0)
	{
		if (position > -700) return -2;
		if (position > -2200) return -10;
		if (position > -2800) return -16;
		return -22;
	}
  else
	{
		if (position < 700) return 2;
		if (position < 2200) return 10;
		if (position < 2800) return 16;
	return 22;
	}
}

// 把目标速度转换为实际的PWM输出
static void Car_SetDifferentialSpeed(int16_t left_speed, int16_t right_speed)
{
	left_speed *= TRACK_PWM_MULTIPLIER;
	right_speed *= TRACK_PWM_MULTIPLIER;
	left_speed = LimitSpeed(left_speed);
	right_speed = LimitSpeed(right_speed);
	/* 实车验证左右响应与原映射相反，因此交换左右轮输出。 */
	robot_speed((uint8_t)right_speed, (uint8_t)left_speed);
}

// 追踪业务函数
void Robot_Traction(void)
{
	Irtracking_Data data;
	int16_t correction;
	int16_t left_speed;
	int16_t right_speed;

	Irtracking_Read(&data);

	correction = GetTrackCorrection(data.position); // 再算纠偏值
	TrackBlackCount = data.black_count;

	// Search using the last known line position when all sensors lose the line.
	if (data.black_count == 0)
	{
 		if (LostCount < TRACK_LOST_STOP_COUNT)
		{
			LostCount++;
			if (LastPosition < 0)
			{
				// 黑线在左侧，向左寻找
				Car_SetDifferentialSpeed(6, TRACK_SEARCH_SPEED);
			}
			else if (LastPosition > 0)
			{
				// 黑线在右侧，向右寻找
				Car_SetDifferentialSpeed(TRACK_SEARCH_SPEED, 6);
			}
			else
			{
				Car_SetDifferentialSpeed(TRACK_CROSS_SPEED, TRACK_CROSS_SPEED);
			}
		}
		else
		{
			Car_SetDifferentialSpeed(0, 0);
		}
		return;
	}

	LostCount = 0;
	data.position = (int16_t)(data.position * 0.6 + LastPosition * 0.4);
	LastPosition = data.position; // 更新上一次位置，一定要写！
	// 十字路口处理：当检测到 6 个或更多传感器检测到黑线时，认为是十字路口。
	if (data.black_count >= 6)
	{
		correction = GetTrackCorrection(data.position);
		LastPosition = data.position;
		left_speed = TRACK_CROSS_SPEED + correction;
		right_speed = TRACK_CROSS_SPEED - correction;
		Car_SetDifferentialSpeed(left_speed, right_speed);
		return;
	}

	// 直接使用当前采样值，避免位置滤波造成纠偏滞后。
	correction = GetTrackCorrection(data.position);
	LastPosition = data.position;

	left_speed = TRACK_BASE_SPEED + correction;
	right_speed = TRACK_BASE_SPEED - correction;
	Car_SetDifferentialSpeed(left_speed, right_speed);
}

int main(void)
{
	uint16_t displayElapsedMs = 0;
	uint16_t lightAO;
	uint8_t lightDO;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	Irtracking_Init();
	robot_Init();
	Encoder_Init();
	LightSensor_Init();
	OLED_Init();
	OLED_ShowString(1, 1, "BLACK:");
	OLED_ShowString(2, 1, "DO:");
	OLED_ShowString(2, 8, "AO:");
	OLED_ShowString(3, 1, "TUNNEL:");

	while (1)
	{
		Robot_Traction();
		/*
		displayElapsedMs += 5;
		if (displayElapsedMs >= 500)
		{
			displayElapsedMs = 0;
			lightDO = LightSensor_ReadDO();
			lightAO = LightSensor_ReadAO();
			OLED_ShowNum(1, 7, TrackBlackCount, 1);
			OLED_ShowNum(2, 4, lightDO, 1);
			OLED_ShowNum(2, 11, lightAO, 4);
			OLED_ShowNum(3, 9, TunnelCount, 2);
		}
		*/
		
		Delay_ms(1);
	}
}
