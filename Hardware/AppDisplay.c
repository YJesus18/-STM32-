#include "AppDisplay.h"
#include "OLED.h"

#define APP_DISPLAY_DISTANCE_PERIOD_MS 100U

static uint16_t DistanceDisplayElapsedMs;

void AppDisplay_Init(void)
{
	OLED_Init();
	DistanceDisplayElapsedMs = 0U;
	OLED_ShowString(1, 1, "BLUE:000 WALK");
	OLED_ShowString(2, 1, "TUN:000 OUT");
	OLED_ShowString(3, 1, "LEN:0000cm");
	OLED_ShowString(4, 1, "DIST:---cm");
}

void AppDisplay_ShowBlueCount(uint8_t count)
{
	OLED_ShowNum(1, 6, count, 3);
}

void AppDisplay_ShowStop(void)
{
	OLED_ShowString(1, 10, "STOP");
}

void AppDisplay_ShowTunnelEntered(void)
{
	OLED_ShowString(2, 9, "IN ");
}

void AppDisplay_UpdateDistance(uint16_t distance_cm, uint8_t valid)
{
	DistanceDisplayElapsedMs++;
	if (DistanceDisplayElapsedMs < APP_DISPLAY_DISTANCE_PERIOD_MS)
	{
		return;
	}

	DistanceDisplayElapsedMs = 0U;
	if (valid != 0U)
	{
		OLED_ShowNum(4, 6, distance_cm, 3);
	}
	else
	{
		OLED_ShowString(4, 6, "---");
	}
}

void AppDisplay_ShowTunnelResult(uint16_t count, uint32_t length_cm)
{
	OLED_ShowNum(2, 5, count, 3);
	OLED_ShowNum(3, 5, length_cm, 4);
	OLED_ShowString(2, 9, "OUT");
}
