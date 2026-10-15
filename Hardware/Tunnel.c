#include "Tunnel.h"
#include "AppConfig.h"
#include "Encoder.h"
#include "LightSensor.h"

static uint16_t TunnelCount;
static uint8_t TunnelInside;
static uint8_t TunnelCandidateDark;
static uint8_t TunnelDebounceCount;
static uint16_t TunnelSampleElapsedMs;
static int32_t TunnelStartLeft;
static int32_t TunnelStartRight;
static uint32_t TunnelLengthPulses;
static uint32_t TunnelLengthCm;
static uint8_t TunnelEnteredEvent;
static uint8_t TunnelResultEvent;

static uint32_t AbsEncoderDelta(int32_t current, int32_t start)
{
	int32_t delta = current - start;
	return (delta < 0) ? (uint32_t)(-delta) : (uint32_t)delta;
}

static void Tunnel_UpdateLength(int32_t left_count, int32_t right_count)
{
	uint32_t left_pulses = AbsEncoderDelta(left_count, TunnelStartLeft);
	uint32_t right_pulses = AbsEncoderDelta(right_count, TunnelStartRight);

	TunnelLengthPulses = (left_pulses + right_pulses) / 2U;
	TunnelLengthCm = (TunnelLengthPulses * 100UL +
		ENCODER_COUNTS_PER_100CM / 2UL) / ENCODER_COUNTS_PER_100CM;
}

static void Tunnel_Sample(void)
{
	uint8_t dark;
	int32_t left_count;
	int32_t right_count;

	dark = (LightSensor_ReadDO() == TUNNEL_DARK_DO_LEVEL) ? 1U : 0U;
	left_count = Encoder_GetLeftCount();
	right_count = Encoder_GetRightCount();

	if (TunnelInside != 0U)
	{
		Tunnel_UpdateLength(left_count, right_count);
	}
	if (dark != TunnelCandidateDark)
	{
		TunnelCandidateDark = dark;
		TunnelDebounceCount = 1U;
		return;
	}
	if (TunnelDebounceCount < TUNNEL_DEBOUNCE_SAMPLES)
	{
		TunnelDebounceCount++;
		return;
	}

	if ((dark != 0U) && (TunnelInside == 0U))
	{
		TunnelInside = 1U;
		TunnelStartLeft = left_count;
		TunnelStartRight = right_count;
		TunnelLengthPulses = 0U;
		TunnelLengthCm = 0U;
		TunnelEnteredEvent = 1U;
	}
	else if ((dark == 0U) && (TunnelInside != 0U))
	{
		Tunnel_UpdateLength(left_count, right_count);
		TunnelInside = 0U;
		TunnelCount++;
		TunnelResultEvent = 1U;
	}
}

void Tunnel_Init(void)
{
	TunnelCount = 0U;
	TunnelInside = 0U;
	TunnelCandidateDark = 0U;
	TunnelDebounceCount = 0U;
	TunnelSampleElapsedMs = 0U;
	TunnelStartLeft = 0;
	TunnelStartRight = 0;
	TunnelLengthPulses = 0U;
	TunnelLengthCm = 0U;
	TunnelEnteredEvent = 0U;
	TunnelResultEvent = 0U;
	Encoder_Init();
	LightSensor_Init();
}

void Tunnel_Update(void)
{
	TunnelSampleElapsedMs++;
	if (TunnelSampleElapsedMs >= TUNNEL_SAMPLE_PERIOD_MS)
	{
		TunnelSampleElapsedMs = 0U;
		Tunnel_Sample();
	}
}

uint8_t Tunnel_IsInside(void)
{
	return TunnelInside;
}

uint8_t Tunnel_HasEntered(void)
{
	uint8_t event = TunnelEnteredEvent;
	TunnelEnteredEvent = 0U;
	return event;
}

uint8_t Tunnel_HasNewResult(void)
{
	uint8_t event = TunnelResultEvent;
	TunnelResultEvent = 0U;
	return event;
}

uint16_t Tunnel_GetCount(void)
{
	return TunnelCount;
}

uint32_t Tunnel_GetLengthCm(void)
{
	return TunnelLengthCm;
}

uint32_t Tunnel_GetLengthPulses(void)
{
	return TunnelLengthPulses;
}
