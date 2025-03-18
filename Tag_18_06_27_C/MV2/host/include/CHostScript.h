// Name:
//	CHostScript.h
//
// Purpose:
//	Handle script execution
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
//	12.09.16 SD - Fix member function name
//				- Change type of Loop in ResultInfos typedef
//
// Copyright (c) 2016 Metrolab Technology SA, Geneva,
//	Switzerland

#ifndef CMV2_HOST_SCRIPT_H
#define CMV2_HOST_SCRIPT_H

// Include files
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlschemas.h>
#include <libxml/xmlwriter.h>

#include <string>
#include <vector>

#include <CMV2HostException.h>

using namespace std;

// Our namespace
namespace MV2Host
{
	// Type definitions
	typedef struct ResultInfos
	{
		bool Average;				// Indicate that commands inside a loop are averaged. Useful only if Loop > 0
		unsigned char Loop;			// Indicate that next commands are inside a loop (Useful when averaging is enabled)
		int NbCommands;				// Indicate number of commands inside a loop. Useful only if Loop > 0.
		int OutputIndex;			// Indicate output index
		string OutputName;			// indicate output name
		ResultInfos(bool Average, bool Loop, int NbCommands, int OutputIndex, string OutputName) :
			Average(Average), Loop(Loop), NbCommands(NbCommands), OutputIndex(OutputIndex), OutputName(OutputName) {}
	}tResultInfos;

	// Result type
	typedef unsigned short tResult;

	// Forward declaration
	class CArduinoSerialPort;

	class CHostScript
	{
	public:

		// Constructor
		CHostScript(
								CArduinoSerialPort 			*pArduino,			// Pointer to the Arduino object
								const char 					*pScriptFileName,	// Script filename
								const char 					*pSchemaFileName);	// Schema filename

		// Destructor
		~CHostScript();

		// Execute initialization script
		void ExecuteInitializationScript();

		// Execute measurement script
		void ExecuteMeasurementScript();

		// Get repeat measurement script
		int GetRepeatMeasurementScript ()
		{
			return m_RepeatMeasurementScript;
		}

		// Get results
		vector< vector<tResult> > GetResults ()
		{
			return m_Results;
		}

		// Get results in CSV format
		string GetCsvResults ()
		{
			return this->ConvertResultsToCSV(this->m_Results);
		}

		// Get headings in CSV format
		string GetCsvHeadings ()
		{
			return this->ConvertHeadingsToCSV(m_Headings);
		}

	private:
		CArduinoSerialPort*			m_pArduino;
		xmlXPathContextPtr 			m_pXPathCtx;
		xmlNodePtr					m_pInitializationScriptNode;
		int							m_RepeatInitializationScript;
		xmlNodePtr					m_pMeasurementScriptNode;
		int							m_RepeatMeasurementScript;
		vector< vector<tResult> > 	m_Results;
		vector<string>				m_Headings;

		// Execute a script
		void Execute (
								vector<tResultInfos> 		&rResultsInfos,		// Informations about results
								vector<unsigned short>		&rCommandsBuffer);	// Commands buffer

		// According to ScriptXPath, check script node
		void CheckScriptNode (
								const xmlChar*				pScriptXPath,		// Pointer to the XPath
								xmlXPathContextPtr			pXPathCtx,			// Pointer to the XPath context
								int							&rRepeat,			// Repeat attribute
								xmlNodePtr					&rpScriptNode);		// Pointer to the script node

		// Compute response index
		void ComputeResponseIndex (
								int							ResponseSize,		// Response size
								int							&rStatusIndex,		// Status index
								int							&rCrcIndex,			// CRC index
								int							&rStatusDescIndex,	// Status description index
								int							&rFirstDataIndex,	// First data index
								unsigned int				&rNbResults);		// Number of results

		// Find maximum output index
		int FindMaxOutputIndex (
								vector<tResultInfos>		ResultsInfos);		// Informations about results

		// Compute Average
		int Average (
								vector<tResult>				Vector);			// Results vector

		// Fill commands buffer from XML nodes
		int FillCommandsBufferFromXmlNodes (
								xmlNodePtr 					pRootNode,			// Pointer to the root node
								xmlXPathContextPtr			pXPathCtx,			// General XPath context of the XML file
								vector<unsigned short>		&rCommandsBuffer,	// Commands buffer
								vector<tResultInfos>		&rResultsInfos);	// Informations about results

		// Create command according to type and value
		unsigned short CreateCommand (
								unsigned char 				CommandType,		// Command type
								unsigned char 				CommandValue);		// Command value

		// Get command from XML node
		void  GetCommandFromXmlNode (
								xmlNodePtr					pCommandNode,		// Pointer to the command node
								xmlXPathContextPtr			pXPathCtx,			// Pointer to the XPath context
								unsigned char				&rCommandType,		// Command type
								unsigned char				&rCommandValue,		// Command value
								int							&rOutputIndex,		// Output index
								string						&rOutputName);		// Output name

		// Parse results
		void ParseResults (
								tResult						*pResponseBuffer,	// Response buffer
								int							ResponseSize,		// Response size
								vector<tResultInfos>		ResultsInfos,		// Informations about results
								vector< vector<tResult> >	&rResults);			// Results

		// Convert Headings in CSV format
		string ConvertHeadingsToCSV (
								vector<string> 				Headings);			// Headings

		// Convert results in CSV format
		string ConvertResultsToCSV (
								vector< vector<tResult> >	Results);			// Results
	}; // CHostScript

} // namespace MV2Host
#endif // CMV2_HOST_SCRIPT_H
