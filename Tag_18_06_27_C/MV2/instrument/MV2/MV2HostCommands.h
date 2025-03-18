// Name:
//	MV2HostCommands.h
//
// Purpose:
// Interface definition between host and Arduino
//
// Description:
//
// Coding Conventions:
//	- Variable names use the "InterCaps" convention
//	- Local variables are prefixed with an underscore, '_'
//	- Pointer variables are prefixed with 'p'
//	- Enumerated types are prefixed with 'e'
//	- Enumerated type values are prefixed with 'k'
//	- Constants are all uppercase
//	The above conventions can also be combined:
//	- A local pointer variable would be prefixed '_p'
//
// Change log:
//	01.02.16 SD	Original version
//	01.03.16 SD - Delete MV2_ERR_CONFLICT error
//				- Fix MV2_CMD_INFO
//	19.04.16 SD Add comments
//	25.04.16 SD Delete errors constants
//  07.07.16 PK Delete kTimeoutError, kUnknownError
//	12.09.16 SD Add GetFwVersion command
//
// Copyright (c) 2016 Metrolab Technology SA, Geneva,
//	Switzerland

#ifndef MV2_HOST_COMMANDS_H
#define MV2_HOST_COMMANDS_H

// Type definition for MV2 command
typedef unsigned char					MV2_CMD;

// Commands
#define MV2_CMD_READ_REGISTER_0			0x1C
#define MV2_CMD_READ_REGISTER_1			0x1D
#define MV2_CMD_READ_REGISTER_2			0x1E
#define MV2_CMD_WRITE_REGISTER_0		0x2C
#define MV2_CMD_WRITE_REGISTER_1		0x2D
#define MV2_CMD_WRITE_REGISTER_2		0x2E
#define MV2_CMD_SET_INIT_BIT			0x01
#define MV2_WAIT_FOR_DR_INTERRUPT		0x02
#define MV2_CMD_DIGITIZE_B_X			0x41
#define MV2_CMD_DIGITIZE_B_Y			0x42
#define MV2_CMD_DIGITIZE_B_Z			0x43
#define MV2_CMD_DIGITIZE_TEMP			0x44			
#define MV2_CMD_SET_OPTIONS				0x45			
#define MV2_CMD_SET_DIGITAL_ANALOG_MODE	0xC1		
#define MV2_CMD_SET_LOOP_START			0xC2
#define MV2_CMD_SET_LOOP_END			0xC3
#define MV2_CMD_GET_FW_VERSION			0xC4

// Enumeration of errors
typedef enum {
	kNoError							= 0,
	kSyntaxError						= 101,
	kModeError							= 102,
	kOutOfMemoryError					= 103,
	kNestedLoopError					= 104,
	kUnspecifiedLoopError				= 105, 
	kBadCrcError						= 201,
	kScriptLengthTooLargeError			= 202,
	kNoValidDataFromHostError			= 203,
	kTransmissionError					= 204,
	kAdcTimeOutError					= 301,
} eError;

// Enumeration of command numbers
typedef enum {
	kReadRegister0 = 0,
	kReadRegister1,
	kReadRegister2,
	kWriteRegister0,
	kWriteRegister1,
	kWriteRegister2,
	kSetInitBit,
	kWaitForDrInterrupt,
	kDigitizeBx,
	kDigitizeBy,
	kDigitizeBz,
	kDigitizeTemp,
	kSetOptions,
	kSetDigitalAnalogMode,
	kSetLoopStart,
	kSetLoopEnd,
	kGetFwVersion
} eCommand;

// Enumeration of command type
typedef enum {
	kDigital = 0,
	kAnalog,
	kMisc
} eCommandType;

// Enumeration of MV2 mode
typedef enum {
	kDigitalMode = 0,
	kAnalogMode
} eMode;


/*
	Structure MV2_CMD_INFO contains all informations about each command
*/
const struct
{
	eCommandType		Type;
	bool				ContainsData;
	bool				ReturnsValue;		
	MV2_CMD				Command;
} MV2_CMD_INFO[] =
{
//		Type		ContainsData	ReturnsValue				MV2_CMD	
	{ kDigital,			false,			true,			MV2_CMD_READ_REGISTER_0			},		// kReadRegister0
	{ kDigital,			false,			true,			MV2_CMD_READ_REGISTER_1			},		// kReadRegister1
	{ kDigital,			false,			true,			MV2_CMD_READ_REGISTER_2			},		// kReadRegister2
	{ kDigital,			true,			true,			MV2_CMD_WRITE_REGISTER_0		},		// kWriteRegister0
	{ kDigital,			true,			true,			MV2_CMD_WRITE_REGISTER_1		},		// kWriteRegister1
	{ kDigital,			true,			true,			MV2_CMD_WRITE_REGISTER_2		},		// kWriteRegister2
	{ kDigital,			true,			false,			MV2_CMD_SET_INIT_BIT			},		// kSetInitBit
	{ kDigital,			false,			false,			MV2_WAIT_FOR_DR_INTERRUPT		},		// kWaitForDrInterrupt
	{ kAnalog,			false,			true,			MV2_CMD_DIGITIZE_B_X			},		// kDigitizeBx
	{ kAnalog,			false,			true,			MV2_CMD_DIGITIZE_B_Y			},		// kDigitizeBy
	{ kAnalog,			false,			true,			MV2_CMD_DIGITIZE_B_Z			},		// kDigitizeBz
	{ kAnalog,			false,			true,			MV2_CMD_DIGITIZE_TEMP			},		// kDigitizeTemp
	{ kAnalog,			true,			false,			MV2_CMD_SET_OPTIONS				},		// kSetOptions
	{ kMisc,			true,			false,			MV2_CMD_SET_DIGITAL_ANALOG_MODE	},		// kSetDigitalAnalogMode
	{ kMisc,			true,			false,			MV2_CMD_SET_LOOP_START			},		// kSetLoopStart
	{ kMisc,			false,			false,			MV2_CMD_SET_LOOP_END			},		// kSetLoopEnd
	{ kMisc,			false,			true,			MV2_CMD_GET_FW_VERSION			}		// kGetFwVersion
};															

/*
	Get command
	Parameters:
		[in]	Command : raw command
		[out]	pCommand : pointer to the command
	Returns:
		eError
*/
eError GetCommand(MV2_CMD Command, eCommand *pCommand);

#endif // MV2_HOST_COMMANDS_H
