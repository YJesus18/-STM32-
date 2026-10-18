#include "VisionControl.h"
#include "Serial.h"

#define VISION_FRAME_HEAD      0xAAU
#define VISION_FRAME_TAIL      0x55U
#define VISION_CMD_BLUE_COUNT  0x01U
#define VISION_CMD_RED_STOP    0x02U

typedef enum
{
	VISION_WAIT_HEAD = 0,
	VISION_WAIT_COMMAND,
	VISION_WAIT_DATA,
	VISION_WAIT_TAIL
} VisionParseState;

static VisionParseState ParseState;
static uint8_t ReceivedCommand;
static uint8_t ReceivedData;
static uint8_t BlueCount;
static uint8_t BlueCountEvent;
static uint8_t RedStopEvent;

void VisionControl_Init(void)
{
	ParseState = VISION_WAIT_HEAD;
	ReceivedCommand = 0U;
	ReceivedData = 0U;
	BlueCount = 0U;
	BlueCountEvent = 0U;
	RedStopEvent = 0U;
	Serial_Init();
}

void VisionControl_Update(void)
{
	uint8_t data;

	while (Serial_ReadByte(&data) != 0U)
	{
		switch (ParseState)
		{
			case VISION_WAIT_HEAD:
				if (data == VISION_FRAME_HEAD)
				{
					ParseState = VISION_WAIT_COMMAND;
				}
				break;

			case VISION_WAIT_COMMAND:
				ReceivedCommand = data;
				ParseState = VISION_WAIT_DATA;
				break;

			case VISION_WAIT_DATA:
				ReceivedData = data;
				ParseState = VISION_WAIT_TAIL;
				break;

			case VISION_WAIT_TAIL:
				if (data == VISION_FRAME_TAIL)
				{
					if ((ReceivedCommand == VISION_CMD_BLUE_COUNT) &&
						(ReceivedData != BlueCount))
					{
						BlueCount = ReceivedData;
						BlueCountEvent = 1U;
					}
					else if ((ReceivedCommand == VISION_CMD_RED_STOP) &&
						(ReceivedData == 1U))
					{
						RedStopEvent = 1U;
					}
				}
				ParseState = (data == VISION_FRAME_HEAD) ?
					VISION_WAIT_COMMAND : VISION_WAIT_HEAD;
				break;

			default:
				ParseState = VISION_WAIT_HEAD;
				break;
		}
	}
}

uint8_t VisionControl_HasNewBlueCount(void)
{
	uint8_t event = BlueCountEvent;
	BlueCountEvent = 0U;
	return event;
}

uint8_t VisionControl_GetBlueCount(void)
{
	return BlueCount;
}

uint8_t VisionControl_HasRedStopRequest(void)
{
	uint8_t event = RedStopEvent;
	RedStopEvent = 0U;
	return event;
}
