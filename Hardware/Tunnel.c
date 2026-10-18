#include "Tunnel.h"
#include "AppConfig.h"
#include "Encoder.h"
#include "Ultrasonic.h"

static uint16_t TunnelCount;
static uint8_t TunnelInside;
static uint8_t TunnelCandidateInside;
static uint8_t TunnelDebounceCount;
static int32_t TunnelStartLeft;
static int32_t TunnelStartRight;
static uint32_t TunnelLengthPulses;
static uint32_t TunnelLengthCm;
static uint16_t TunnelDistanceCm;
static uint8_t TunnelDistanceValid;
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

static void Tunnel_ProcessUltrasonic(void)
{
	uint8_t detected_inside;
	uint8_t distance_valid;
	uint16_t distance_cm;
	int32_t left_count;
	int32_t right_count;

	distance_valid = Ultrasonic_IsValid();
	distance_cm = Ultrasonic_GetDistanceCm();
	TunnelDistanceValid = distance_valid;
	if (distance_valid != 0U)
	{
		TunnelDistanceCm = distance_cm;
	}
	left_count = Encoder_GetLeftCount();
	right_count = Encoder_GetRightCount();

	if (TunnelInside != 0U)
	{
		detected_inside = ((distance_valid != 0U) &&
			(distance_cm < TUNNEL_EXIT_DISTANCE_CM)) ? 1U : 0U;
	}
	else
	{
		detected_inside = ((distance_valid != 0U) &&
			(distance_cm <= TUNNEL_ENTER_DISTANCE_CM)) ? 1U : 0U;
	}

	if (TunnelInside != 0U)
	{
		Tunnel_UpdateLength(left_count, right_count);
	}
	if (detected_inside != TunnelCandidateInside)
	{
		TunnelCandidateInside = detected_inside;
		TunnelDebounceCount = 1U;
		return;
	}
	if (TunnelDebounceCount < TUNNEL_CONFIRM_SAMPLES)
	{
		TunnelDebounceCount++;
	}
	if (TunnelDebounceCount < TUNNEL_CONFIRM_SAMPLES) return;

	if ((detected_inside != 0U) && (TunnelInside == 0U))
	{
		TunnelInside = 1U;
		TunnelStartLeft = left_count;
		TunnelStartRight = right_count;
		TunnelLengthPulses = 0U;
		TunnelLengthCm = 0U;
		TunnelEnteredEvent = 1U;
	}
	else if ((detected_inside == 0U) && (TunnelInside != 0U))
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
	TunnelCandidateInside = 0U;
	TunnelDebounceCount = 0U;
	TunnelStartLeft = 0;
	TunnelStartRight = 0;
	TunnelLengthPulses = 0U;
	TunnelLengthCm = 0U;
	TunnelDistanceCm = 0U;
	TunnelDistanceValid = 0U;
	TunnelEnteredEvent = 0U;
	TunnelResultEvent = 0U;
	Encoder_Init();
	Ultrasonic_Init();
}

void Tunnel_Update(void)
{
	Ultrasonic_Update();

	if (Ultrasonic_HasNewMeasurement() != 0U)
	{
		Tunnel_ProcessUltrasonic();
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

uint16_t Tunnel_GetDistanceCm(void)
{
	return TunnelDistanceCm;
}

uint8_t Tunnel_IsDistanceValid(void)
{
	return TunnelDistanceValid;
}
