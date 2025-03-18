// Name:
//	main.cpp
//
// Purpose:
//	Main entry point for Arduino MagVectorMV2 host software
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
//	02.11.15 SD	Original version
//	09.03.16 SD Work continues
//	15.03.16 SD Work continues
//	16.03.16 SD Work continues
//	24.03.16 SD - Handle OS type for read and write to MV2
//				- Improve script execution
//				- Delete redefinition of O_NOCITY
//				- Handle OS type for some output
//				- Fix bug in ReadFromMV2 function for *nix OS
//	29.03.16 SD - Code cleanup
//				- Fix format for errno for *NIX OS
//
//	30.03.16 SD - Add CeckCrc function
//				- Fix bug when handle OS type
//	31.03.16 SD Configure serial port settings
//  01.04.16 SD Initialize serial port settings
//	07.04.16 SD Refactoring serial port (Opening, Settings,...). Create functions in HostUtility.h
//	19.04.16 SD After opening serial port, *NIX OS reboot Arduino so we have to wait
//	22.04.16 SD Handle MXR file
//	27.04.16 SD Clean up
//	28.04.16 SD Move several routines to HostResults.cpp and HostScript.cpp
//	23.05.16 SD Handle modifications of MXR utility routines
//	16.08.16 SD Code review
//	12.09.16 SD Display software version in usage function
//	11.07.17 PK	Catch SIGINT in order to cleanly shut down serial port
//				Add code to catch ^C in Windows envirnment
//	25.05.20 PK	Add __CYGWIN__ for MSYS2
//
// Copyright (c) 2016 Metrolab Technology SA, Geneva,
//	Switzerland

#include <iostream>

#include <CMxrFile.h>
#include <CArduinoSerialPort.h>
#include <CHostScript.h>
#include <MV2HostSoftwareVersion.h>
#include <CMV2HostException.h>

// The technique for catching a ^C is different in Windows (of course...)
#if defined(_WIN32) || defined(_WIN32_WCE) || defined(__WIN32__) || defined(__CYGWIN__)
#  ifndef WIN32
#    define WIN32
#  endif
#endif

#ifdef WIN32
	#include <windows.h>
#else
	#include <csignal>
#endif

using namespace MV2Host;

// Display usage informations
void usage(const char *pName)
{
	// Display version
	cout << "Version " << MV2HOST_SOFTWARE_VERSION_MAJOR << "." << MV2HOST_SOFTWARE_VERSION_MINOR << endl;
	// Display usage
	cout << "Usage: " << pName << " <MV2ScriptXml-file> <MV2ScriptSchemaXsd-file> <COM port> [MXR-file]" << endl;
}

// Catch SIGINT signal (^C).
#ifdef WIN32
	volatile static bool _InterruptReceived = 0;
	BOOL WINAPI signal_handler(DWORD dwType)
	{
		switch(dwType)
		{
			case CTRL_C_EVENT:
			case CTRL_BREAK_EVENT:
				cout << "^C";
				_InterruptReceived = 1;
				return TRUE;
			default:
				return FALSE;
		}
	}
#else
	volatile static std::sig_atomic_t _InterruptReceived = 0;
	static void signal_handler(int _Signal)
	{
		_InterruptReceived = 1;
	}
#endif

// Main program
int main(int argc, char **argv)
{
	// Check command line
	// Argument 5 is optional (MXR file)
	if ((argc < 4) || (argc > 5))
	{
		cerr << "Error: wrong number of arguments." << endl;
		usage(argv[0]);
		return(-1);
	}

	// Catch exceptions.
	try
	{
	// Set up the handler to handle ^C.
#ifdef WIN32
		if (!SetConsoleCtrlHandler ((PHANDLER_ROUTINE)signal_handler, TRUE))
			throw CMV2HostException("Cannot install ^C handler");
#else
		if (std::signal(SIGINT, signal_handler) == SIG_ERR)
			throw CMV2HostException("Cannot install ^C handler");
#endif

		// Check if optional argument 5 (MXR filename) is present
		CMxrFile *_pMxrFile = NULL;
		if (argc == 5)
			_pMxrFile = new CMxrFile(argv[4]);

		// Create CArduinoSerialPort object
		CArduinoSerialPort *_pArduino = new CArduinoSerialPort(argv[3]);

		// Create CHostScript object
		CHostScript *_pHostScript = new CHostScript(_pArduino, argv[1], argv[2]);

		// Execute initialization script
		_pHostScript->ExecuteInitializationScript();

		// Execute measurement script
		for (int _RepeatCounter = 0;
				(_pHostScript->GetRepeatMeasurementScript() == 0) || (_RepeatCounter < _pHostScript->GetRepeatMeasurementScript());
				_RepeatCounter++)
		{
			// Check whether we received ^C.
			if (_InterruptReceived)
			{
				cout << "Interrupt received!\n";
				break;
			}
			
			// Execute measurement script
			_pHostScript->ExecuteMeasurementScript();
			
			// Display results
			cout << _pHostScript->GetCsvResults().c_str();
			
			// Write results to the MXR file
			if (_pMxrFile != NULL)
				_pMxrFile->WriteResults(_pHostScript->GetCsvResults().c_str(), _pHostScript->GetCsvHeadings().c_str());
		}

		// Clean up memory
		delete _pHostScript;
		delete _pArduino;
		if (_pMxrFile != NULL)
			delete _pMxrFile;
	}
	catch (CMV2HostException & rE)
	{
		cerr << "Error: " << rE.what() << endl;
		return -1;
	}
	catch (...)
	{
		cerr << "Error : an unexpected exception has occurred." << endl;
		return -1;
	}

	return 0;
}
