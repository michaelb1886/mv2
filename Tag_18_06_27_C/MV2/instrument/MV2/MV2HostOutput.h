// Name:
//	MV2HostOutput.h
//
// Purpose:
// Fetch and format results
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
//	01.02.16 SD	Original version
//	01.03.16 SD Move constants to MV2HostConstants.h
//	19.04.16 SD Delete global variable
//
// Copyright (c) 2016 Metrolab Technology SA, Geneva,
//	Switzerland

#ifndef MV2_HOST_OUTPUT_H
#define MV2_HOST_OUTPUT_H

#include "MV2HostCommands.h"
#include "MV2Utility.h"
#include "MV2HostConstants.h"

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
void SendResponse(uint16_t *pResponseBuffer, uint16_t NumberOfResults, eError Error, uint16_t ErrorDesc);

#endif //MV2_HOST_OUTPUT_H
