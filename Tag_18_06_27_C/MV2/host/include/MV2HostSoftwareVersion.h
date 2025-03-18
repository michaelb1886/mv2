// Name:
//	MV2HostSoftwareVersion.h
//
// Purpose:
//	Handle MV2 Host software version
//
// Description:
//
// Coding Conventions:
//	- Variable names use the "InterCaps" convention
//	- Local variables are prefixed with an underscore, '_'
//	- Private class members are prefixed with 'm_'
//	- Private class methods are prefixed with an underscore, '_'
//	- Pointer variables are prefixed with 'p'
//	- Reference variables are prefixed with 'r'
//	- Enumerated types are prefixed with 'e'
//	- Enumerated type values are prefixed with 'k'
//	- Classes are prefixed with 'C'
//	- Constants are all uppercase
//	The above conventions can also be combined:
//	- A local pointer variable would be prefixed '_p'
//	- A private class pointer member would be prefixed 'm_p'
//
// Change log:
//	07.09.16 SD	Original version
//	03.04.17 PK	Bump the version: Adapt for use with Arduino MEGA 2560
//	03.04.17 PK	Bump the version: Catch ^C and close serial port cleanly; open COMx for x>9
//	21.08.17 PK Bump the version: Reset Arduino by enabling DTR in Windows
//
// Copyright (c) 2016 Metrolab Technology SA, Geneva,
//	Switzerland


#define MV2HOST_SOFTWARE_VERSION_MAJOR	1
#define MV2HOST_SOFTWARE_VERSION_MINOR	3
