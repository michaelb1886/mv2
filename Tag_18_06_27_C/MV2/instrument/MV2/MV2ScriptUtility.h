// Name:
//	MV2ScriptUtility.h
//
// Purpose:
// Utility function to handle script	
//
// Description:
// Handle script execution, check and execute commands
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

#ifndef MV2_SCRIPT_UTILIY_H
#define MV2_SCRIPT_UTILIY_H

#include "Arduino.h"
#include "MV2HostCommands.h"
#include "MV2Hal.h"

eError ExecuteScript(uint16_t *pScriptBuffer,
	uint16_t sizeScriptBuffer,
	uint16_t *pOutputBuffer,
	uint16_t sizeOutputBuffer,
	uint16_t *pNbEltOutputBuffer,
	uint16_t *pIndexScriptError);

#endif // MV2_SCRIPT_UTILIY_H