#include "TrackControl.h"
#include "AppConfig.h"
#include "Irtracking.h"
#include "robot.h"

static int16_t LastPosition;
static uint16_t LostCount;
static uint8_t TrackBlackCount;
static uint8_t TrackStopped;

static int16_t LimitSpeed(int16_t speed)
{
	if (speed > 100) return 100;
	if (speed < 0) return 0;
	return speed;
}


/* 使用原来的分段纠偏逻辑，不使用PID。 */
/*static int16_t GetTrackCorrection(int16_t position)
{
	if ((position > -160) && (position < 160))
	{
		return 0;
	}

	if (position < 0)
	{
		if (position > -700) return -9;
		if (position > -2200) return -12;
		if (position > -2800) return -18;
		return -5;
	}

	if (position < 700) return 9;
	if (position < 2200) return 12;
	if (position < 2800) return 18;
	return 5;
}*/

static int16_t GetTrackCorrection(int16_t position)
{
	if (position < 0) return -TRACK_TWO_SENSOR_CORRECTION;
	if (position > 0) return TRACK_TWO_SENSOR_CORRECTION;
	return 0;
}

static void Car_SetDifferentialSpeed(int16_t left_speed, int16_t right_speed)
{
	left_speed *= TRACK_PWM_MULTIPLIER;
	right_speed *= TRACK_PWM_MULTIPLIER;
	left_speed = LimitSpeed(left_speed);
	right_speed = LimitSpeed(right_speed);
	/* 实车验证左右响应与原映射相反，因此交换左右轮输出。 */
	robot_speed((uint8_t)right_speed, (uint8_t)left_speed);
}

void TrackControl_Init(void)
{
	LastPosition = 0;
	LostCount = 0U;
	TrackBlackCount = 0U;
	TrackStopped = 0U;
	Irtracking_Init();
	robot_Init();
}

// 循迹业务函数
void TrackControl_Update(void)
{
	Irtracking_Data data;
	int16_t correction;
	int16_t left_speed;
	int16_t right_speed;

	if (TrackStopped != 0U)
	{
		robot_speed(0U, 0U);
		return;
	}

	Irtracking_Read(&data);
	TrackBlackCount = data.black_count;

	if (data.black_count == 0U)
	{
		if (LostCount < TRACK_LOST_STOP_COUNT)
		{
			LostCount++;
			if (LastPosition < 0)
			{
				Car_SetDifferentialSpeed(0, TRACK_SEARCH_SPEED);
			}
			else if (LastPosition > 0)
			{
				Car_SetDifferentialSpeed(TRACK_SEARCH_SPEED, 0);
			}
			else
			{
				Car_SetDifferentialSpeed(TRACK_BASE_SPEED, TRACK_BASE_SPEED);
			}
		}
		else
		{
			Car_SetDifferentialSpeed(0, 0);
		}
		return;
	}

	LostCount = 0U;
	LastPosition = data.position;
	correction = GetTrackCorrection(data.position);
	left_speed = TRACK_BASE_SPEED + correction;
	right_speed = TRACK_BASE_SPEED - correction;
	Car_SetDifferentialSpeed(left_speed, right_speed);
}

uint8_t TrackControl_GetBlackCount(void)
{
	return TrackBlackCount;
}

void TrackControl_Stop(void)
{
	TrackStopped = 1U;
	robot_speed(0U, 0U);
}
