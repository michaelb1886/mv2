// Name:
//	CArduinoSerialPort.cpp
//
// Purpose:
//	See CArduinoSerialPort.h
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
//	12.09.16 SD Improve exception description message with GetLastErrorStdStr function
//	12.07.17 PK	Allow for serial ports larger than COM9 (Windows)
//	21.08.17 PK Reset Arduino by enabling DTR in Windows
//
// Copyright (c) 2016 Metrolab Technology SA, Geneva,
//	Switzerland

// Include files
#include <CArduinoSerialPort.h>
#include <iostream>
#include <string.h>

// Exceptions messages
#define OPENING_SERIAL_PORT_EXCEPTION_MSG		"CArduinoSerialPort: Unable to open serial port."
#define GET_SERIAL_PORT_SETTINGS_EXCEPTION_MSG	"CArduinoSerialPort: Unable to get serial port settings."
#define SET_SERIAL_PORT_SETTINGS_EXCEPTION_MSG	"CArduinoSerialPort: Unable to set serial port settings."
#define FLUSH_SERIAL_PORT_EXCEPTION_MSG			"CArduinoSerialPort: Unable to flush serial port settings."
#define WRITE_SERIAL_PORT_EXCEPTION_MSG			"CArduinoSerialPort: Unable to write to the serial port."
#define READ_SERIAL_PORT_EXCEPTION_MSG			"CArduinoSerialPort: Unable to read serial port."
#define BAD_CRC_EXCEPTION_MSG					"CArduinoSerialPort: Bad CRC."

// Our namespace
namespace MV2Host
{

// Create a string with last error message
string GetLastErrorStdStr()
{
	#ifdef WIN32
		DWORD _Error = GetLastError();
		if (_Error)
		{
			LPVOID lpMsgBuf;
			DWORD bufLen = FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				_Error,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf,
				0, NULL );
			if (bufLen)
			{
			  LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
			  string result(lpMsgStr, lpMsgStr+bufLen);

			  LocalFree(lpMsgBuf);

			  return result;
			}
		}
		return string();
	#else
		return string (strerror(errno));
	#endif
}

// Constructor
CArduinoSerialPort::CArduinoSerialPort(const char *pPortName)
{
	#ifdef WIN32
		// Prefix portname with "\\\\.\\".
		// (see https://support.microsoft.com/en-us/help/115831/howto-specify-serial-ports-larger-than-com9)
		char _PortName[20] = "\\\\.\\";				// 20 allows for portnames up to COM999999999
		strncat(_PortName, pPortName, 12);
		
		m_PortHandle = CreateFile(_PortName,		// Specify port device
			GENERIC_READ | GENERIC_WRITE,			// Specify mode that open device.
			0,										// the device isn't shared.
			0,										// the object gets a default security.
			OPEN_EXISTING,							// Specify which action to take on file.
			FILE_ATTRIBUTE_NORMAL,					// default.
			0);										// default.
		if (m_PortHandle == INVALID_HANDLE_VALUE)
		{
			string _ErrorMsg = OPENING_SERIAL_PORT_EXCEPTION_MSG + GetLastErrorStdStr();
			throw CMV2HostException(_ErrorMsg);
		}
	#else
		m_PortHandle = open(pPortName, O_RDWR | O_NOCTTY );
		if (m_PortHandle < 0)
		{
			string _ErrorMsg = OPENING_SERIAL_PORT_EXCEPTION_MSG + GetLastErrorStdStr();
			throw CMV2HostException(_ErrorMsg);
		}
		// *NIX OS reboot Arduino so we have to wait
		sleep(WAIT_FOR_ARDUINO_REBOOT);
	#endif

		// Set serial port settings
		SetSerialPortSettings();
} // Constructor

// Destructor
CArduinoSerialPort::~CArduinoSerialPort()
{
#ifdef WIN32
	CloseHandle(m_PortHandle);
#else
	close(m_PortHandle);
#endif
} // Destructor

// Set serial port settings 8N1, 57600 bauds
void CArduinoSerialPort::SetSerialPortSettings()
{
#ifdef WIN32
	DCB _DcbSerialParams;

	// Get serial port settings
	if (!GetCommState(m_PortHandle, &_DcbSerialParams))
	{
		string _ErrorMsg = GET_SERIAL_PORT_SETTINGS_EXCEPTION_MSG + GetLastErrorStdStr();
		throw CMV2HostException(_ErrorMsg);
	}

	_DcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;
	_DcbSerialParams.BaudRate = CBR_57600;
	_DcbSerialParams.ByteSize = 8;
	_DcbSerialParams.StopBits = ONESTOPBIT;
	_DcbSerialParams.Parity = NOPARITY;

	// Set serial port settings. Resets Arduino, because DTR is set.
	if (!SetCommState(m_PortHandle, &_DcbSerialParams))
	{
		string _ErrorMsg = SET_SERIAL_PORT_SETTINGS_EXCEPTION_MSG + GetLastErrorStdStr();
		throw CMV2HostException(_ErrorMsg);
	}
	
	// Wait for the reset.
	Sleep(WAIT_FOR_ARDUINO_REBOOT);

	// Flush anything already in the serial buffer
	if (!PurgeComm(m_PortHandle, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR))
	{
		string _ErrorMsg = FLUSH_SERIAL_PORT_EXCEPTION_MSG + GetLastErrorStdStr();
		throw CMV2HostException(_ErrorMsg);
	}
#else
	struct termios _PortSettings;

	// Get serial port settings
	if (tcgetattr(m_PortHandle, &_PortSettings) != 0)
	{
		string _ErrorMsg = GET_SERIAL_PORT_SETTINGS_EXCEPTION_MSG + GetLastErrorStdStr();
		throw CMV2HostException(_ErrorMsg);
	}
	 // Set baud rates
	cfsetispeed(&_PortSettings, B57600);
	cfsetospeed(&_PortSettings, B57600);

	// Set 8N1
	_PortSettings.c_cflag &= ~PARENB;
	_PortSettings.c_cflag &= ~CSTOPB;
	_PortSettings.c_cflag &= ~CSIZE;
	_PortSettings.c_cflag |= CS8;

	// Enable receiver, ignore status line
	_PortSettings.c_cflag |= (CREAD | CLOCAL | CRTSCTS);
	// Diable canonical input, disable echo, disable visually erase chars,
	// disable terminal-generated signals
	_PortSettings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	// Disable hardware flow control
	_PortSettings.c_cflag &= ~CRTSCTS;
	// Disable input/output flow control, disable restart chars
	_PortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);
	// Disable output processing
	_PortSettings.c_oflag &= ~OPOST;
	// Configured timed read
	_PortSettings.c_cc[VMIN] = 0;
	// 20s
	_PortSettings.c_cc[VTIME] = 200;
	// Set serial port settings
	if (tcsetattr(m_PortHandle, TCSANOW, &_PortSettings) != 0)
	{
		string _ErrorMsg = SET_SERIAL_PORT_SETTINGS_EXCEPTION_MSG + GetLastErrorStdStr();
		throw CMV2HostException(_ErrorMsg);
	}
	// Flush anything already in the serial buffer
	if (tcflush(m_PortHandle, TCIFLUSH) != 0)
	{
		string _ErrorMsg = FLUSH_SERIAL_PORT_EXCEPTION_MSG + GetLastErrorStdStr();
		throw CMV2HostException(_ErrorMsg);
	}	

#endif
} // SetSerialPortSettings

// Write script buffer to Arduino
void CArduinoSerialPort::Write (	unsigned short	*pBuffer,		// Buffer to write
									unsigned int	Size)			// Size of buffer
{
	NoOfBytes_t _NoOfBytesWritten = 0;
#ifdef WIN32
	if (!WriteFile(m_PortHandle, pBuffer, Size, &_NoOfBytesWritten, NULL))
	{
		string _ErrorMsg = WRITE_SERIAL_PORT_EXCEPTION_MSG + GetLastErrorStdStr();
		throw CMV2HostException(_ErrorMsg);
	}
#else
	_NoOfBytesWritten = write(m_PortHandle, pBuffer, Size);
	if (_NoOfBytesWritten < 0)
	{
		string _ErrorMsg = WRITE_SERIAL_PORT_EXCEPTION_MSG + GetLastErrorStdStr();
		throw CMV2HostException(_ErrorMsg);
	}
#endif
} // Write

// Low level read
void CArduinoSerialPort::LowLevelRead (	unsigned char	*pBuffer,
										NoOfBytes_t		NoOfBytesToRead,
										NoOfBytes_t		&rNoOfBytesRead)
{
	NoOfBytes_t _BytesRead = 0;
	rNoOfBytesRead = 0;
	NoOfBytes_t _RemainingBytes = NoOfBytesToRead;

	// Loop until all the bytes are received
	while (rNoOfBytesRead < NoOfBytesToRead)
	{
#ifdef WIN32
		if(!ReadFile(m_PortHandle, &pBuffer[rNoOfBytesRead], _RemainingBytes, &_BytesRead, NULL))
		{
			string _ErrorMsg = READ_SERIAL_PORT_EXCEPTION_MSG + GetLastErrorStdStr();
			throw CMV2HostException(_ErrorMsg);
		}
#else
		_BytesRead = read(m_PortHandle, &pBuffer[rNoOfBytesRead], _RemainingBytes);
		// Check error
		if (_BytesRead < 0)
		{
			string _ErrorMsg = READ_SERIAL_PORT_EXCEPTION_MSG + GetLastErrorStdStr();
			throw CMV2HostException(_ErrorMsg);
		}
#endif
		rNoOfBytesRead += _BytesRead;
		_RemainingBytes -= _BytesRead;
	}
} // Low level read

// Read response buffer from Arduino
void CArduinoSerialPort::Read (	unsigned short	*pBuffer,
								NoOfBytes_t		&rNoOfBytesRead)
{
	unsigned char *_pTmpBuffer = (unsigned char *)pBuffer;
	NoOfBytes_t _BytesRead = 0;
	rNoOfBytesRead = 0;

	// Get header
	LowLevelRead(&_pTmpBuffer[0], RESPONSE_HEADER_LENGTH * sizeof(unsigned short), _BytesRead);

	// Update bytes read
	rNoOfBytesRead = _BytesRead;

	// Compute bytes to read according to pBuffer[0]
	int _BytesToRead = pBuffer[0] - _BytesRead;

	// Get remaining response according to header pBuffer[0]
	LowLevelRead(&_pTmpBuffer[_BytesRead], _BytesToRead, _BytesRead);

	// Update bytes read
	rNoOfBytesRead += _BytesRead;
} // Read

// Write commands buffer to the serial port and read response
void CArduinoSerialPort::WriteAndRead(	vector<unsigned short>	&rCommandsBuffer,			// Commands buffer
										tResult					*pResponseBuffer,			// Response buffer
										unsigned int			&rResponseBufferSize)		// Size of response buffer
{
	// Insert size at index 0
	rCommandsBuffer.insert(rCommandsBuffer.begin(), (rCommandsBuffer.size()  +
														SCRIPT_BUFFER_HEADER_LENGTH +
															SCRIPT_BUFFER_CRC_LENGTH)
																* sizeof(unsigned short));
	// Generate and insert CRC
	rCommandsBuffer.push_back(GenerateCrc(rCommandsBuffer.size(), &rCommandsBuffer[0]));

	// Write command buffer to the Arduino
	Write(&rCommandsBuffer[0], rCommandsBuffer[0]);

	// Read response from Arduino
	NoOfBytes_t _BytesRead;
	Read(pResponseBuffer, _BytesRead);

	// Compute size of response buffer
	rResponseBufferSize = _BytesRead/sizeof(tResult);

	// Check CRC
	if (!CheckCrc(_BytesRead/sizeof(tResult), pResponseBuffer))
		throw CMV2HostException(BAD_CRC_EXCEPTION_MSG);
} // WriteAndRead

// Generate CRC
unsigned short CArduinoSerialPort::GenerateCrc (unsigned short	Size,			// Size of message
												unsigned short	*pMessage)		// Message
{
	unsigned short _Result = 0;

	for (unsigned short _i=0; _i<Size; _i++)
		_Result ^= pMessage[_i];

	return _Result;
} // GenerateCrc

// Check CRC
bool CArduinoSerialPort::CheckCrc (	int				Size,		// Size of message
									unsigned short	*pMessage)	// Message
{
	unsigned short _Result = 0;

	for (int _i=0; _i<Size; _i++)
		_Result ^= pMessage[_i];

	return (_Result == 0);
} // CheckCrc
} // namespace MV2Host
