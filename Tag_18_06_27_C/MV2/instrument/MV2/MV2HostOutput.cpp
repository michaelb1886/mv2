// Name:
//	MV2HostOutput.cpp
//
// Purpose:
// Fetch and format results
//
// Description:
// See MV2HostOutput.h
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
//	25.04.16 SD Handle error code only with eError
//
// Copyright (c) 2016 Metrolab Technology SA, Geneva,
//	Switzerland

#define _GLOBAL_RESPONSE_

#include "MV2HostOutput.h"

/*
	Send Response to the host
	Parameters:
		[in]		pResponseBuffer : pointer to the first element of the response buffer
		[in]		NumberOfResults : size of the response (bytes)
		[in]		Error : error code
		[in]		ErrorDesc : error description
	Returns:
		void
*/
void SendResponse(uint16_t *pResponseBuffer, uint16_t NumberOfResults, eError Error, uint16_t ErrorDesc)
{
	
	// Compute response length in bytes
	uint16_t _ResponseLength =	RESPONSE_HEADER_LENGTH	+
								NumberOfResults			+
								RESPONSE_STATUS_LENGTH	+
								RESPONSE_CRC_LENGTH;

	// Add response size to the header
	pResponseBuffer[0] = _ResponseLength * sizeof(uint16_t);

	// Compute index
	uint16_t _IndexErrorCode	= RESPONSE_HEADER_LENGTH + NumberOfResults;
	uint16_t _IndexErrorDesc	= RESPONSE_HEADER_LENGTH + NumberOfResults + 1;
	uint16_t _IndexCrc			= RESPONSE_HEADER_LENGTH + NumberOfResults + RESPONSE_STATUS_LENGTH;

	// Error code
	pResponseBuffer[_IndexErrorCode] = static_cast<uint16_t>(Error);

	// Error description
	pResponseBuffer[_IndexErrorDesc] = ErrorDesc;

	// Generate CRC
	pResponseBuffer[_IndexCrc] = GenerateCrc(pResponseBuffer, _IndexCrc);
	
	// Send response to the host
	for (uint16_t _i = 0; _i < _ResponseLength; _i++)
	{
		Serial.write(pResponseBuffer[_i]);
		Serial.write(pResponseBuffer[_i] >> 8);
	}
}