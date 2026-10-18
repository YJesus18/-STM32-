#include "stm32f10x.h"
#include "Delay.h"
#include "TrackControl.h"
#include "Tunnel.h"
#include "AppDisplay.h"
#include "VisionControl.h"

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	TrackControl_Init();
	Tunnel_Init();
	AppDisplay_Init();
	VisionControl_Init();

	while (1)
	{
		VisionControl_Update();
		if (VisionControl_HasRedStopRequest() != 0U)
		{
			TrackControl_Stop();
			AppDisplay_ShowStop();
		}

		TrackControl_Update();
		Tunnel_Update();
		AppDisplay_UpdateDistance(
			Tunnel_GetDistanceCm(),
			Tunnel_IsDistanceValid());

		if (VisionControl_HasNewBlueCount() != 0U)
		{
			AppDisplay_ShowBlueCount(VisionControl_GetBlueCount());
		}

		if (Tunnel_HasEntered() != 0U)
		{
			AppDisplay_ShowTunnelEntered();
		}

		if (Tunnel_HasNewResult() != 0U)
		{
			AppDisplay_ShowTunnelResult(
				Tunnel_GetCount(),
				Tunnel_GetLengthCm());
		}

		Delay_ms(5);
	}
}
