// Name:
//	MV2HostCommands.cpp
//
// Purpose:
// Interface definition between host and Arduino
//
// Description:
// See MV2HostCommands.cpp
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
//	19.04.16 SD Add SIZE_OF_MV2_CMD_INFO constant
//
// Copyright (c) 2016 Metrolab Technology SA, Geneva,
//	Switzerland

#include "MV2HostCommands.h"

#define SIZE_OF_MV2_CMD_INFO sizeof(MV2_CMD_INFO) / sizeof(MV2_CMD_INFO[0])

/*
	Get command
	Parameters:
		[in]	Command : raw command
		[out]	pCommand : pointer to the command
	Returns:
		eError
*/
eError GetCommand(MV2_CMD Command, eCommand *pCommand)
{
	for (unsigned short _i = 0; _i < SIZE_OF_MV2_CMD_INFO; _i++)
	{
		if (MV2_CMD_INFO[_i].Command == Command)
		{
			*pCommand = static_cast<eCommand>(_i);
			return kNoError;
		}
	}
	return kSyntaxError;
}