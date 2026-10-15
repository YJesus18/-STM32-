#include "stm32f10x.h"
#include "Delay.h"
#include "TrackControl.h"
#include "Tunnel.h"
#include "AppDisplay.h"

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	TrackControl_Init();
	Tunnel_Init();
	AppDisplay_Init();

	while (1)
	{
		TrackControl_Update();
		Tunnel_Update();

		if (Tunnel_HasEntered() != 0U)
		{
			AppDisplay_ShowTunnelEntered();
		}

		if (Tunnel_HasNewResult() != 0U)
		{
			AppDisplay_ShowTunnelResult(
				Tunnel_GetCount(),
				Tunnel_GetLengthCm(),
				Tunnel_GetLengthPulses());
		}

		Delay_ms(1);
	}
}
