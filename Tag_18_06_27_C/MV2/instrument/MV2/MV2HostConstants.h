// Name:
//	MV2HostConstants.h
//
// Purpose:
//	Constants definit
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
//	25.02.16 SD	Original version
//	07.07.16 SD Fix comments
//  02.04.17 PK Increase MAX_RESPONSE_LENGTH for Arduino MEGA
//
// Copyright (c) 2016 Metrolab Technology SA, Geneva,
//	Switzerland

#ifndef MV2_HOST_CONSTANTS_H
#define MV2_HOST_CONSTANTS_H

/* Script buffer is defined as follows:
--------------------
|     HEADER        | 1 word
--------------------
|     SCRIPT        | BUFFER_LENGTH words
--------------------
|     CRC           | 1 word
--------------------
*/
// Define script buffer constants. Expressed as 16-bit word
#define SCRIPT_BUFFER_LENGTH					64
#define SCRIPT_BUFFER_HEADER_LENGTH				1
#define SCRIPT_BUFFER_CRC_LENGTH				1
#define SCRIPT_BUFFER_ADDITIONAL_INFOS_LENGTH	(SCRIPT_BUFFER_HEADER_LENGTH + SCRIPT_BUFFER_CRC_LENGTH)

// Define transfer timeout
#define HOST_TO_MV2_TRANSFER_LONG_TIMEOUT		100000000
#define HOST_TO_MV2_TRANSFER_SHORT_TIMEOUT		2000

/* Response is defined as follows:
--------------------
|     HEADER        | 1 word
--------------------
|     RESULTS       | RESULTS_LENGTH words
--------------------
|     STATUS        | 2 words
--------------------
|     CRC           | 1 word
--------------------
*/
// Define constant. Expressed as 16-bits word.
// Note: should leave 512B free (check by setting DEBUG to 1 in MV2.ino).
#if defined(__AVR_ATmega328P__)     // UNO
    #define MAX_RESPONSE_LENGTH                        500 
#elif defined(__AVR_ATmega2560__)   // MEGA 2560
    #define MAX_RESPONSE_LENGTH                        3565 
#else
    #error "Unknown board"
#endif
#define RESPONSE_HEADER_LENGTH					1
#define RESPONSE_STATUS_LENGTH					2
#define RESPONSE_CRC_LENGTH						1
#define MAX_RESULTS_LENGTH						(MAX_RESPONSE_LENGTH-RESPONSE_HEADER_LENGTH-RESPONSE_STATUS_LENGTH-RESPONSE_CRC_LENGTH)

#endif // MV2_HOST_CONSTANTS_H
