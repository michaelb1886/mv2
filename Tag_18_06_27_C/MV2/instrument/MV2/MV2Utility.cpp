// Name:
//	MV2Utility.cpp
//
// Purpose:
// Utility function
//
// Description:
// See MV2Utility.h
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
//
// Copyright (c) 2016 Metrolab Technology SA, Geneva,
//	Switzerland

#include "MV2Utility.h"

/*
	Generate CRC
	Parameters:
		[in]		pMessage	: pointer to the message
		[in]		Size		: size of the message
	Returns:
		uint16_t
*/

uint16_t GenerateCrc(uint16_t *pMessage, uint16_t Size)
{
	uint16_t _Result = 0;

	for (uint16_t _i = 0; _i<Size; _i++)
		_Result ^= pMessage[_i];

	return _Result;
}

/*
	Check CRC
	Parameters:
		[in]		pBuffer		: pointer to the buffer
		[in]		Size		: size of the buffer
	Returns:
		bool
*/
bool CheckCrc(uint16_t *pBuffer, uint16_t Size)
{
	uint16_t _Result = 0;

	for (uint16_t _i = 0; _i<Size; _i++)
		_Result ^= pBuffer[_i];

	return (_Result == 0);
}

/*
    Return free memory
    Parameters:
    Returns:
        int
*/
int freeRam (void)
{
    extern int __heap_start, *__brkval; 
    int v; 
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
