// Name:
//	MV2FirmwareVersion.h
//
// Purpose:
//	Handle MV2 firmware version
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
//	07.09.16 SD	Original version
//  16.03.17 PK Bump firmware version: do not wait for DR in write&read
//  24.03.17 PK Bump firmware version: After INIT=1, toggle CS to clear spurious DR
//  31.03.17 PK Bump firmware version: Add support for Arduino MEGA
//  22.08.17 PK Bump firmware version: Fix bug switching from digital to serial mode
//  11.09.18 PK Bump firmware version: Slow down SPI bit rate, to allow for long cables
//
// Copyright (c) 2016 Metrolab Technology SA, Geneva,
//	Switzerland

#define FW_VERSION 0x0105
