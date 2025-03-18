// Name:
//	CArduinoSerialPort.h
//
// Purpose:
//	Handle serial communication with Arduino
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
//	16.08.16 SD	Original version
//	03.04.17 PK	Adapt for use with Arduino MEGA 2560
//	21.08.17 PK	Define WAIT_FOR_ARDUINO_REBOOT for Windows
//	25.05.20 PK	Add __CYGWIN__ for MSYS2
//
// Copyright (c) 2016 Metrolab Technology SA, Geneva,
//	Switzerland

#ifndef CARDUINO_SERIAL_PORT_H
#define CARDUINO_SERIAL_PORT_H

#if defined(_WIN32) || defined(_WIN32_WCE) || defined(__WIN32__) || defined(__CYGWIN__)
#  ifndef WIN32
#    define WIN32
#  endif
#endif

// Include files
#include <vector>

// Set up to use Arduino MEGA 2560 - worst-case for memory usage
#define __AVR_ATmega2560__

#include <MV2HostConstants.h>

#ifdef WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <fcntl.h>
	#include <errno.h>
	#include <termios.h>
#endif

#ifdef WIN32
	#define WAIT_FOR_ARDUINO_REBOOT 2000
	typedef HANDLE File_t;
	typedef DWORD NoOfBytes_t;
#else
	#define WAIT_FOR_ARDUINO_REBOOT 2
	typedef int File_t;
	typedef ssize_t NoOfBytes_t;
#endif

#include <CMV2HostException.h>
#include <CHostScript.h>

using namespace std;

// Our namespace
namespace MV2Host
{
	class CArduinoSerialPort
	{
	public:
		// Constructor
		CArduinoSerialPort (
									const char 					*pPortName);				// Port name
		// Destructor
		~CArduinoSerialPort ();

		// Write commands buffer to the serial port and read response
		void WriteAndRead (
									vector<unsigned short>		&rCommandsBuffer,			// Commands buffer
									tResult						*pResponseBuffer,			// Response buffer
									unsigned int				&rResponseBufferSize);		// Size of response buffer

	private:
		File_t						m_PortHandle;											// Port handle

		// Set serial port settings
		void SetSerialPortSettings ();

		// Write to the serial port
		void Write (
									unsigned short				*pBuffer,					// Buffer to write
									unsigned int				Size);						// Size of buffer

		// Read serial port (low-level)
		void LowLevelRead (
									unsigned char				*pBuffer,					// Buffer
									NoOfBytes_t					NoOfBytesToRead,			// Number of bytes to read
									NoOfBytes_t					&rNoOfBytesRead);			// Number of bytes read

		// Read serial port
		void Read (
									unsigned short				*pBuffer,					// Buffer
									NoOfBytes_t					&rNoOfBytesRead);			// Number of bytes read

		// Generate CRC
		unsigned short GenerateCrc (
									unsigned short				Size,						// Size of message
									unsigned short				*pMessage);					// Message
		// Check CRC
		bool CheckCrc (
									int Size,												// Size of message
									unsigned short				*pMessage);					// Message
	}; // CArduinoSerialPort
} // namespace MV2Host
#endif // CARDUINO_SERIAL_PORT_H
