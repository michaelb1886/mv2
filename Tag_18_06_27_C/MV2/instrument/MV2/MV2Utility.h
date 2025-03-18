// Name:
//	MV2Utility.h
//
// Purpose:
// Utility function
//
// Description:
// Handle CRC 
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
//  03.04.17 PK Add freeRam
//
// Copyright (c) 2016 Metrolab Technology SA, Geneva,
//	Switzerland

#ifndef MV2_UTILIY_H
#define MV2_UTILIY_H

#include "Arduino.h"

uint16_t GenerateCrc(uint16_t *pMessage, uint16_t Size);
bool CheckCrc(uint16_t *pBuffer, uint16_t Size);
int freeRam (void);


#endif // MV2_UTILIY_H
