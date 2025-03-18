// Name:
//	MV2ScriptUtility.cpp
//
// Purpose:
// Utility function to handle script	
//
// Description:
// See MV2ScriptUtility.h
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
//
// Change log:
//	01.02.16 SD	Original version
//	01.03.16 SD Fix bugs
//	19.04.16 SD Add comments
//	06.07.16 SD Handle error with commands kWriteRegister0, kWriteRegister1 and kWriteRegister2 in ExecuteCommand function
//	12.09.16 SD Handle kGetFwVersion command
//  03.04.17 PK Add freeRam
//
// Copyright (c) 2016 Metrolab Technology SA, Geneva,
//	Switzerland

#include "MV2ScriptUtility.h"
#include "MV2FirmwareVersion.h"

/*
	Execute command
	Parameters:
		[in]		Command		: command to execute
		[in]		CommandVal	: command parameter
		[out]		pRetVal		: pointer to the return value
	Returns:
		eError
*/
eError ExecuteCommand(eCommand Command, uint8_t CommandVal, uint16_t *pRetVal)
{
	eError _Error = kNoError;

	switch (Command)
	{
		// kReadRegister* commands, command value is in the command itself
		case kReadRegister0:
		case kReadRegister1:
		case kReadRegister2:
			*pRetVal = DigitalReadRegister(MV2_CMD_INFO[Command].Command);
			break;

		case kWriteRegister0:
		case kWriteRegister1:
		case kWriteRegister2:
			_Error = DigitalWriteAndRead(MV2_CMD_INFO[Command].Command << 8 | CommandVal, pRetVal);
			break;

		case kSetInitBit:
			DigitalSetInitBit((CommandVal == 0) ? LOW : HIGH);
			break;

		case kWaitForDrInterrupt:
			_Error = DigitalWaitForDataReady();
			break;

		case kDigitizeBx:
			*pRetVal = AnalogDigitizeBx();
			break;

		case kDigitizeBy:
			*pRetVal = AnalogDigitizeBy();
			break;

		case kDigitizeBz:
			*pRetVal = AnalogDigitizeBz();
			break;

		case kDigitizeTemp:
			*pRetVal = AnalogDigitizeTemp();
			break;

		case kSetOptions:
				AnalogSetOptions(CommandVal);
			break;

		case kSetDigitalAnalogMode:
				MiscSetDigitalAnalogMode( (CommandVal == 0) ? kDigitalMode : kAnalogMode);
			break;

		// No action to perform
		case kSetLoopStart:
		case kSetLoopEnd:
			break;
			
		case kGetFwVersion:
			*pRetVal = FW_VERSION;
			break;

		default:
			_Error = kSyntaxError;
			break;
	}
	return _Error;
}

/*
	Check Command
	Parameters:
		[in]		Command : raw command to check
		[out]		pCmd : if command is correctly checked, *pCmd contains command
	Returns:
		eError
*/
eError CheckCommand(uint8_t Command, eCommand *pCmd)
{
	eError _Error = kNoError;

	// Check if command exists
	if ( (_Error = GetCommand(Command, pCmd)) != kNoError)
		return _Error;

	// Get MV2 mdoe
	eMode _MV2Mode = GetMV2Mode();
	// Safety check
	// Analog command must be executed only in analog mode
	if ((_MV2Mode == kDigitalMode) && (MV2_CMD_INFO[*pCmd].Type == kAnalog))
		return kModeError;
	// Digital command must be executed only in digital mode
	else if ((_MV2Mode == kAnalogMode) && (MV2_CMD_INFO[*pCmd].Type == kDigital))
		return kModeError;
	else
		return kNoError;
}

/*
	SearchForEndLoopIndex
	Parameters:
		[in]		pCommandsBuffer : commands buffer
		[in]		Length : script buffer size
		[in]		IndexStart : index to start search
		[out]		pIndexEndLoop : if found, index of end loop
	Returns:
		eError
*/
eError SearchForEndLoopIndex(	uint16_t *pCommandsBuffer,
								uint16_t Length,
								uint16_t IndexStart,
								uint16_t *pIndexEndLoop)
{
	eError _Error = kNoError;
	eCommand _Cmd;
	*pIndexEndLoop = -1;

	for (uint16_t _i = IndexStart; _i < Length; _i++)
	{
		// Get command
		_Error = GetCommand(pCommandsBuffer[_i] >> 8, &_Cmd);
		// Handle error
		if (_Error != kNoError)
			break;
		// Handle error
		else
		{
			if (_Cmd == kSetLoopEnd)
			{
				*pIndexEndLoop = _i;
				break;
			}
			if (_Cmd == kSetLoopStart)
			{
				*pIndexEndLoop = _i;
				_Error = kNestedLoopError;
				break;
			}
		}
	}
	if (*pIndexEndLoop == -1)
		_Error = kUnspecifiedLoopError;

	return _Error;
}

/*
	Execute script
	Parameters:
		[in/out]	pCommandsBuffer : pointer to the first element of the commands array
		[in]		CommandsBufferLength : length of commands array
		[in/out]	pOutputBuffer : pointer to the output buffer
		[in]		ResultsBufferLength : length of results buffer
		[in/out]	pResultsBufferIndex : index of the results buffer
		[out]		pIndexCommandError : index where an error has occured
	Returns:
		eError
*/
eError ExecuteScript (	uint16_t *pCommandsBuffer,
						uint16_t CommandsBufferLength,
						uint16_t *pOutputBuffer,
						uint16_t ResultsBufferLength,
						uint16_t *pResultsBufferIndex,
						uint16_t *pIndexCommandError)
{
	// Initialize error
	eError _Error = kNoError;
	// Current command
	eCommand _Cmd;
	// Current command value
	uint8_t _CmdValue;
	// Command return value
	uint16_t _CmdRetVal;
	// Handle loop start and end index
	uint16_t _IndexLoopStart = 0;
	uint16_t _IndexEndLoop;
	// Index of the command where an error occured
	*pIndexCommandError = 0;

	// Main loop
	for (uint16_t _i = 0; _i < CommandsBufferLength; _i++)
	{
		// Get command value
		_CmdValue =  (uint8_t)pCommandsBuffer[_i] & 0xFF;

		// Check Command
		_Error = CheckCommand(pCommandsBuffer[_i] >> 8, &_Cmd);

		// Handle error
		if (_Error != 0)
		{
			*pIndexCommandError = _i;
			return _Error;
		}
		
		// Loop detected
		else if (_Cmd == kSetLoopStart)
		{
			// Ckeck loop and search for end loop index
			_IndexLoopStart = _i + 1;
			_Error = SearchForEndLoopIndex(pCommandsBuffer, CommandsBufferLength, _IndexLoopStart, &_IndexEndLoop);
			// Handle error
			if (_Error != kNoError)
			{
				*pIndexCommandError = _i;
				return _Error;
			} // Handle error
			// Execute loop
			else
			{
				// Loop
				for (uint16_t _j = 0; _j < _CmdValue; _j++)
				{
					// Execute script
					_Error = ExecuteScript(	&pCommandsBuffer[_IndexLoopStart],
											_IndexEndLoop - _IndexLoopStart,
											pOutputBuffer,
											ResultsBufferLength,
											pResultsBufferIndex,
											pIndexCommandError);
					// Handle any errors
					if (_Error != kNoError)
					{
						*pIndexCommandError = _i;
						return _Error;
					}
				} // Loop
				// Update current main loop index _i to next index after end loop
				_i = _IndexEndLoop;
			} // Execute loop
		} // Loop detected
		// Execute command
		else
		{
			// Execute command
			_Error = ExecuteCommand(_Cmd, _CmdValue, &_CmdRetVal);
			// Handle error
			if (_Error != kNoError)
				return _Error;
			// Append command return value to the output buffer
			else
			{
				// Check if command returns a value
				if (MV2_CMD_INFO[_Cmd].ReturnsValue)
				{
					// Add command response to the output buffer
					// Check memory
					if (*pResultsBufferIndex < ResultsBufferLength)
					{
						pOutputBuffer[*pResultsBufferIndex] = _CmdRetVal;
						(*pResultsBufferIndex)++;
					} // Check memory
					// Out of memory error
					else
						return kOutOfMemoryError;
				} // Check if command returns a value
			} // Append command return value to the output buffer
		} // Execute command
	} // Main loop

	return _Error;
}

