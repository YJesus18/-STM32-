#include "AppDisplay.h"
#include "OLED.h"

void AppDisplay_Init(void)
{
	OLED_Init();
	OLED_ShowString(1, 1, "TUNNEL:000");
	OLED_ShowString(2, 1, "LEN:0000 cm");
	OLED_ShowString(3, 1, "PULSE:00000");
	OLED_ShowString(4, 1, "STATE:OUT");
}

void AppDisplay_ShowTunnelEntered(void)
{
	OLED_ShowString(4, 7, "IN ");
}

void AppDisplay_ShowTunnelResult(uint16_t count,
	uint32_t length_cm, uint32_t pulses)
{
	OLED_ShowNum(1, 8, count, 3);
	OLED_ShowNum(2, 5, length_cm, 4);
	OLED_ShowNum(3, 7, pulses, 5);
	OLED_ShowString(4, 7, "OUT");
}
