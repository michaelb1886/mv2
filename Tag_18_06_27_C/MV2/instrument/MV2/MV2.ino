// Name:
//	main.ino
//
// Purpose:
//	Main entry point for Arduino MagVectorMV2 firmware
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
//
// Change log:
//	22.01.16 SD	Original version
//	01.03.16 SD Fix bugs
//	19.04.16 SD - Add comments
//				- Clear serial buffer when check size generate an error
//				- Add local variables in loop function
//	25.04.16 SD - Change _ScriptBuffer name to _pScriptBuffer
//				- Fix ReadDataFromHost prototype
//	17.06.16 SD MV2 doesn't accept a switch from analog to digital mode. SPI communication is broken.
//				At startup, MV2 mode is now set to digital.
//	07.07.16 SD Fix previous comment : Switch from analog to digital mode works, INV analog bit must be set to default
//  03.04.17 PK List free memory
//
// Copyright (c) 2016 Metrolab Technology SA, Geneva,
//	Switzerland

#include <SPI.h> //Librairie SPI
#include "MV2HostCommands.h"
#include "MV2HostOutput.h"
#include "MV2ScriptUtility.h"
#include "MV2Utility.h"
#include "MV2Hal.h"
#include "MV2HostConstants.h"

#define DEBUG 0

/*
	Forward declaration
*/

eError ReadDataFromHost(uint8_t *pBuffer, uint16_t Size, long TimeOut);

/*
	Initialization
*/
void setup()
{
	// At startup MV2 mode is set to digital
	MiscSetDigitalAnalogMode(kDigitalMode);

	// Initialize serial communication
	Serial.begin(57600, SERIAL_8N1);
}

/*
	Loop forever
*/
void loop()
{
	// Error
	eError _Error = kNoError;
	// Script buffer, cast transmission buffer to uint16_t. Assume Endianness is little endian both on Arduino and Host.
	uint16_t _pScriptBuffer[SCRIPT_BUFFER_LENGTH];
	// Pointer to the commands buffer
	uint16_t *_pCommandsBuffer = &_pScriptBuffer[SCRIPT_BUFFER_HEADER_LENGTH];
	// Response buffer
	uint16_t _pResponse[MAX_RESPONSE_LENGTH];
	// pointer to the results buffer
	uint16_t *_pResultsBuffer = &_pResponse[RESPONSE_HEADER_LENGTH];

    // Debug: list free memory.
#if DEBUG
    Serial.print ("freeRam()=");
    Serial.println (freeRam());
#endif

	// Wait for a new message. First, get header with the transfer length
	_Error = ReadDataFromHost(	reinterpret_cast<uint8_t*>(&_pScriptBuffer[0]),
								SCRIPT_BUFFER_HEADER_LENGTH * sizeof(uint16_t),
								HOST_TO_MV2_TRANSFER_LONG_TIMEOUT);

	// No error 
	if (_Error == kNoError)
	{
		// Check size
		if (_pScriptBuffer[0] > (SCRIPT_BUFFER_LENGTH * sizeof(uint16_t)))
		{
			// Clear serial buffer
			while (Serial.available() > 0)
				Serial.read();
			// Send response with error code
			SendResponse(_pResponse, 0, kScriptLengthTooLargeError, _pScriptBuffer[0]);
		}
		else
		// Read script
		{
			// Read data from host
			_Error = ReadDataFromHost(	reinterpret_cast<uint8_t*>(_pCommandsBuffer),
										_pScriptBuffer[0] - SCRIPT_BUFFER_HEADER_LENGTH * sizeof(uint16_t),
										HOST_TO_MV2_TRANSFER_SHORT_TIMEOUT);
			// Handle transmission error
			if (_Error != kNoError)
				SendResponse(_pResponse, 0, _Error, 0);
			// Check CRC
			else if (!CheckCrc(&_pScriptBuffer[0], _pScriptBuffer[0] / sizeof(uint16_t)))
				SendResponse(_pResponse, 0, kBadCrcError, 0);
			// If CRC is ok, execute script
			else
			{
				uint16_t _NumberOfResults = 0;
				uint16_t _IndexCommandError = 0;
				uint16_t _CommandsNb = _pScriptBuffer[0] / sizeof(uint16_t) - SCRIPT_BUFFER_ADDITIONAL_INFOS_LENGTH;

				// Execute script
				eError _Error = ExecuteScript(	_pCommandsBuffer,
												_CommandsNb,
												_pResultsBuffer,
												MAX_RESULTS_LENGTH,
												&_NumberOfResults,
												&_IndexCommandError);
				// Send response to the host
				SendResponse(_pResponse, _NumberOfResults, _Error, _IndexCommandError);
			} // If CRC is ok
		} // Read script
		
	} // No error 
	else
		SendResponse(_pResponse, 0, _Error, 0);
}

/*
	Read data from host
	Parameters:
		[in/out]	pBuffer	: Pointer to the first element of the buffer
		[in]		Size	: Buffer's size
		[in]		TimeOut	: Time out
	Returns:
		eError
*/
eError ReadDataFromHost(uint8_t *pBuffer, uint16_t Size, long TimeOut)
{
	uint16_t _NbBytesRead = 0;
	// Set timeout
	Serial.setTimeout(TimeOut);
	// Read bytes
	_NbBytesRead = Serial.readBytes(pBuffer, Size);
	// Test _NbBytesRead
	if (_NbBytesRead < 0)
		return kNoValidDataFromHostError;
	else if (_NbBytesRead < Size)
		return kTransmissionError;
	else
		return kNoError;
}
