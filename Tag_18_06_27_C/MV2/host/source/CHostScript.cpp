// Name:
//	CHostScript.cpp
//
// Purpose:
//	See CHostScript.h
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
//	12.09.16 SD - Improve conversion from numerical type to string in ConvertResultsToCSV() 
//				and Execute()
//				- Fix truncation error in Average method
//	03.04.17 PK	Adapt for use with Arduino MEGA 2560
//
// Copyright (c) 2016 Metrolab Technology SA, Geneva,
//	Switzerland

// Set up to use Arduino MEGA 2560 - worst-case for memory usage
#define __AVR_ATmega2560__

// Include files
#include <MV2HostCommands.h>
#include <MV2HostConstants.h>
#include <CArduinoSerialPort.h>
#include <CHostScript.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <map>

// Exception messages
#define COUNT_ATTR_EXCEPTION_MSG				"CHostScript: Attribute count doesn't exists for loop element.\n"
#define PARSE_EXCEPTION_MSG						"CHostScript: Unable to parse results.\n"
#define OPEN_XML_SCHEMA_EXCEPTION_MSG			"CHostScript: Unable to open XML schema.\n"
#define PARSE_XML_FILE_EXCEPTION_MSG			"CHostScript: Unable to parse file XML file.\n"
#define CREATE_XPATH_EVAL_CONTEXT_EXCEPTION_MSG "CHostScript: Unable to create XPath evaluation context.\n"
#define XML_FILE_NOT_VALID_EXCEPTION_MSG		"CHostScript: XML file is not valid.\n"
#define EVAL_XPATH_EXPR_EXCEPTION_MSG			"CHostScript: Unable to evaluate XPath expression.\n"
#define NUMBER_OF_SCRIPT_NODE_EXCEPTION_MSG		"CHostScript: Only one script node is handled.\n"
#define INVALIDE_SCRIPT_NODE_EXCEPTION_MSG		"CHostScript: Invalid script node.\n"
#define GET_NODE_CONTENT_EXCEPTION_MSG			"CHostScript: Unable to get node content.\n"
#define GET_PROPERTY_NODE_EXCEPTION_MSG			"CHostScript: Unable to get property node.\n"
#define PARSE_XML_FILE_AVER_ATTR_EXCEPTION_MSG	"CHostScript: Unable to parse XML file. Attribute Average doesn't exists for loop element.\n"
#define COMMAND_TYPE_EXCEPTION_MSG				"CHostScript: Command type doesn't exist: "
#define MV2_EXCEPTION_MSG						"CHostScript: MV2 error: "
#define COMPUTE_RESPONSE_INDEX_EXCEPTION_MSG	"CHostScript: Unable to compute response index.\n"

// Error messages from Arduino
static map<unsigned int, string> gResponseErrorCodes =
		{
				{kSyntaxError,					"Syntax error"				},
				{kModeError,					"Error mode"				},
				{kOutOfMemoryError,				"Out of memory"				},
				{kNestedLoopError,				"Nested loop"				},
				{kUnspecifiedLoopError,			"Unspecified loop error"	},
				{kBadCrcError,					"Bad CRC"					},
				{kScriptLengthTooLargeError,	"Script length too large"	},
				{kNoValidDataFromHostError,		"No valid data from host"	},
				{kTransmissionError,			"Transmission error"		},
				{kAdcTimeOutError,				"ADC timeout"				}
		};


// Constants for XML script
#define COMMAND_NODE_NAME						"command"
#define LOOP_NODE_NAME							"loop"
#define OUTPUT_INDEX_ATTRIBUTE_NAME				"outputIndex"
#define OUTPUT_NAME_ATTRIBUTE_NAME				"outputName"
#define COUNT_LOOP_ATTRIBUT_NAME				"count"
#define AVERAGE_LOOP_ATTRIBUT_NAME				"average"
#define COMMAND_VALUE_XPATH						".//value"
#define COMMAND_TYPE_XPATH						".//type"
#define REPEAT_ATTIBUTE_NAME					"repeat"

// XPath constants for XML script
#define INITIALIZATION_SCRIPT_XPATH				"/scripts/initialization"
#define MEASUREMENT_SCRIPT_XPATH				"/scripts/measurement"

// Miscellaneous constants
#define HEADING_DEFAULT_PREFIX_NAME				"unknown"

// Response Buffer constants
#define RESPONSE_MINIMUM_LENGTH 				(RESPONSE_HEADER_LENGTH + \
													RESPONSE_STATUS_LENGTH + \
														RESPONSE_CRC_LENGTH)

// Our namespace
namespace MV2Host
{

// Constructor
CHostScript::CHostScript(
							CArduinoSerialPort *pArduino,	// Pointer to the Arduino object
							const char *pScriptFileName,	// Script filename
							const char *pSchemaFileName)	// Schema filename
{
	// Update Arduino
	m_pArduino = pArduino;

	// Initialize libxml
	xmlInitParser();

	// Open XML schema
	xmlSchemaParserCtxtPtr _pSchemaParserCtxt = NULL;
	_pSchemaParserCtxt = xmlSchemaNewParserCtxt(pSchemaFileName);

	// Parse XML schema
	xmlSchemaPtr _pSchema = NULL;
	_pSchema = xmlSchemaParse(_pSchemaParserCtxt);
	if (_pSchema == NULL)
		throw CMV2HostException(OPEN_XML_SCHEMA_EXCEPTION_MSG);

	// Parse XML file
	xmlDocPtr _pXmlDoc = NULL;
	_pXmlDoc = xmlParseFile(pScriptFileName);

	// Check XML file
	if (_pXmlDoc == NULL)
		throw CMV2HostException(PARSE_XML_FILE_EXCEPTION_MSG);

	// Load XML schema
	xmlSchemaValidCtxtPtr _pValidCtxt = NULL;
	_pValidCtxt = xmlSchemaNewValidCtxt(_pSchema);
	xmlSchemaSetValidErrors(_pValidCtxt,
							(xmlSchemaValidityErrorFunc) fprintf,
							(xmlSchemaValidityWarningFunc) fprintf,
							stderr);

	// Validate XML file with XML schema
	xmlSchemaSetValidOptions(_pValidCtxt, XML_SCHEMA_VAL_VC_I_CREATE);
	int _Ret = xmlSchemaValidateDoc(_pValidCtxt, _pXmlDoc);
	if (_Ret > 0)
		throw CMV2HostException(XML_FILE_NOT_VALID_EXCEPTION_MSG);

	// Create XPath evaluation context
	m_pXPathCtx = xmlXPathNewContext(_pXmlDoc);

	// Check XPath evaluation context
	if(m_pXPathCtx == NULL)
		throw CMV2HostException(CREATE_XPATH_EVAL_CONTEXT_EXCEPTION_MSG);

	CheckScriptNode((const xmlChar*)INITIALIZATION_SCRIPT_XPATH, m_pXPathCtx, m_RepeatInitializationScript, m_pInitializationScriptNode);
	CheckScriptNode((const xmlChar*)MEASUREMENT_SCRIPT_XPATH, m_pXPathCtx, m_RepeatMeasurementScript, m_pMeasurementScriptNode);
} // Constructor

// Destructor
CHostScript::~CHostScript()
{
	xmlXPathFreeContext(m_pXPathCtx);
	xmlCleanupParser();
} // Destructor

// According to ScriptXPath, check script node
void CHostScript::CheckScriptNode(
									const xmlChar*		pScriptXPath,		// Pointer to the XPath script
									xmlXPathContextPtr	pXPathCtx,			// Pointer to the XPath context
									int					&rRepeat,			// Repeat attribute
									xmlNodePtr			&rpScriptNode)		// Pointer to the script node
{
	xmlXPathObjectPtr _pXPathObj;

	// Evaluate XPath expression
	_pXPathObj = xmlXPathEvalExpression(pScriptXPath, pXPathCtx);
	if(_pXPathObj == NULL)
		throw CMV2HostException(EVAL_XPATH_EXPR_EXCEPTION_MSG);

	// Check node
	xmlNodeSetPtr _pScriptNodeSet =  _pXPathObj->nodesetval;
	if (_pScriptNodeSet->nodeNr != 1)
		throw CMV2HostException(NUMBER_OF_SCRIPT_NODE_EXCEPTION_MSG);

	// Get script node pointer
	xmlNodePtr _ScriptNode = _pScriptNodeSet->nodeTab[0];

	// Get repeat attribute if it exists
	xmlChar *_TempRepeat = xmlGetProp(_ScriptNode, (const xmlChar*)REPEAT_ATTIBUTE_NAME);
	if (_TempRepeat != NULL)
		rRepeat = strtol((char*)_TempRepeat, NULL, 10);
	else
		rRepeat = -1;
	xmlFree(_TempRepeat);

	// Get script children
	rpScriptNode = _ScriptNode->children;

	// Cleanup
	xmlXPathFreeObject(_pXPathObj);
} // CheckScriptNode

// Execute initialization script
void CHostScript::ExecuteInitializationScript()
{

	vector<unsigned short> _CommandsBuffer;
	vector<tResultInfos> _ResultsInfos;

	FillCommandsBufferFromXmlNodes(m_pInitializationScriptNode, m_pXPathCtx, _CommandsBuffer, _ResultsInfos);

	Execute(_ResultsInfos, _CommandsBuffer);
} // ExecuteInitializationScript

// ExecuteMeasurementScript
void CHostScript::ExecuteMeasurementScript()
{
	vector<unsigned short> _CommandsBuffer;
	vector<tResultInfos> _ResultsInfos;

	FillCommandsBufferFromXmlNodes(m_pMeasurementScriptNode, m_pXPathCtx, _CommandsBuffer, _ResultsInfos);

	Execute(_ResultsInfos, _CommandsBuffer);
} // ExecuteMeasurementScript

// Execute a script
void CHostScript::Execute(	vector<tResultInfos> 		&rResultsInfos,		// Informations about results
							vector<unsigned short>		&rCommandsBuffer)	// Commands buffer
{
	// Response buffer
	tResult _ResponseBuffer[MAX_RESPONSE_LENGTH];

	// Clear results
	m_Results.clear();

	// Response buffer size
	unsigned int _ResponseBufferSize;

	// Send script to the Arduino and wait for the response
	m_pArduino->WriteAndRead(rCommandsBuffer, _ResponseBuffer, _ResponseBufferSize);

	// Parse results
	ParseResults(_ResponseBuffer, _ResponseBufferSize, rResultsInfos, m_Results);

	// Make sure headings are initialized
	m_Headings.clear();
	m_Headings.reserve(m_Results.size());

	// Generate default names for all column
	for (unsigned int _i=0; _i<m_Results.size(); _i++)
	{
		stringstream _Ss;
		_Ss << _i;
		m_Headings.push_back(string(HEADING_DEFAULT_PREFIX_NAME + _Ss.str() ));
	}

	// Change default name if necessary
	for (unsigned int _i=0; _i<rResultsInfos.size(); _i++)
	{
		if ((rResultsInfos[_i].OutputIndex >=0 ) && (strcmp(rResultsInfos[_i].OutputName.c_str(), HEADING_DEFAULT_PREFIX_NAME)))
		{
			m_Headings[rResultsInfos[_i].OutputIndex] = rResultsInfos[_i].OutputName;
		}
	}
} // Execute

// Create command according to type and value
unsigned short CHostScript::CreateCommand(	unsigned char CommandType,		// Command type
											unsigned char CommandValue)		// Command value
{
	return (unsigned short)(CommandType << 8 | (CommandValue & 0xFF));
} // CreateCommand

// Get command type, value and output index from XML node
void  CHostScript::GetCommandFromXmlNode(
								xmlNodePtr			pCommandNode,			// Pointer to the command node
								xmlXPathContextPtr	pXPathCtx,				// Pointer to the XPath context
								unsigned char		&rCommandType,			// Command type
								unsigned char		&rCommandValue,			// Command value
								int					&rOutputIndex,			// Output index
								string				&rOutputName)			// Output name
{
	xmlXPathObjectPtr _pXPathObj;

	// Get type node according to XPath
	_pXPathObj = xmlXPathNodeEval(pCommandNode, (const xmlChar*)COMMAND_TYPE_XPATH, pXPathCtx );

	// Command type node must be equal to one
	if (_pXPathObj->nodesetval->nodeNr != 1)
		throw CMV2HostException(INVALIDE_SCRIPT_NODE_EXCEPTION_MSG);

	// Get and check command node type
	xmlChar *_NodeContent = xmlNodeGetContent(_pXPathObj->nodesetval->nodeTab[0]);
	if (!_NodeContent)
		throw CMV2HostException(GET_NODE_CONTENT_EXCEPTION_MSG);

	// Convert _NodeContent to integer value
	rCommandType = strtol ((const char*)_NodeContent,NULL,16);
	xmlFree(_NodeContent);

	// Free XPath object
	xmlXPathFreeObject(_pXPathObj);

	// Get value node according to XPath
	_pXPathObj = xmlXPathNodeEval(pCommandNode, (const xmlChar*)COMMAND_VALUE_XPATH, pXPathCtx );

	// Command value node must be equal to one
	if (_pXPathObj->nodesetval->nodeNr != 1)
		throw CMV2HostException(INVALIDE_SCRIPT_NODE_EXCEPTION_MSG);

	// Get and check command node value
	_NodeContent = xmlNodeGetContent(_pXPathObj->nodesetval->nodeTab[0]);
	if (!_NodeContent)
		throw CMV2HostException(GET_NODE_CONTENT_EXCEPTION_MSG);

	// Convert _NodeContent to integer value
	rCommandValue = strtol ((const char*)_NodeContent,NULL,16);
	xmlFree(_NodeContent);

	// Get and check output index
	xmlChar *_TempOutputIndex = xmlGetProp(pCommandNode, (const xmlChar *)OUTPUT_INDEX_ATTRIBUTE_NAME);
	if (!_TempOutputIndex)
		throw CMV2HostException(GET_PROPERTY_NODE_EXCEPTION_MSG);

	// Convert _TempOutputIndex to integer value
	rOutputIndex = strtol((char*)_TempOutputIndex, NULL, 10);
	xmlFree(_TempOutputIndex);

	xmlChar *_TempOutputName = xmlGetProp(pCommandNode, (const xmlChar *)OUTPUT_NAME_ATTRIBUTE_NAME);
	if (!_TempOutputName)
		throw CMV2HostException(GET_PROPERTY_NODE_EXCEPTION_MSG);

	// Update output name
	rOutputName.append( (char*)_TempOutputName);

	// Free temperature
	xmlFree(_TempOutputName);

	// Free XPath object
	xmlXPathFreeObject(_pXPathObj);
} // GetCommandFromXmlNode

// Fill commands buffer from XML nodes
int CHostScript::FillCommandsBufferFromXmlNodes (
													xmlNodePtr 				pRootNode,			// Pointer to the root node
													xmlXPathContextPtr		pXPathCtx,			// General XPath context of the XML file
													vector<unsigned short>	&rCommandsBuffer,	// Commands buffer
													vector<tResultInfos>	&rResultsInfos)		// Informations about results
{
	// Store size of commands buffer
	int _OldSize = rCommandsBuffer.size();

	// Loop over nodes
    while (pRootNode != NULL)
    {
    	unsigned char _CommandType;
    	unsigned char _CommandValue;
    	int _OutputIndex;
    	string _OutputName;

    	if (pRootNode->type == XML_ELEMENT_NODE)
    	{
    		// Command Node
    		if (!xmlStrcmp(pRootNode->name, (const xmlChar *)COMMAND_NODE_NAME))
    		{
    			GetCommandFromXmlNode(pRootNode, pXPathCtx, _CommandType, _CommandValue, _OutputIndex, _OutputName);

				// Check if command exists
				eCommand _Cmd;
				if(GetCommand(_CommandType, &_Cmd) == kNoError)
				{
					// Loop's commands are handled with nodes
					if ( (MV2_CMD_INFO[_Cmd].Command != MV2_CMD_SET_LOOP_START) ||  (MV2_CMD_INFO[_Cmd].Command != MV2_CMD_SET_LOOP_END))
					{
						// If command returns a value, save _OutputIndex to handle results from MV2
						if (MV2_CMD_INFO[_Cmd].ReturnsValue)
							rResultsInfos.push_back(tResultInfos(false, 0, 0, _OutputIndex, _OutputName));
						// Add command to the buffer
						rCommandsBuffer.push_back(CreateCommand(_CommandType, _CommandValue));
					}
				}
				else
				{
					char _ErrorBuffer [10];
					snprintf (_ErrorBuffer, sizeof(_ErrorBuffer), "0x%x", _CommandType);
					throw CMV2HostException(COMMAND_TYPE_EXCEPTION_MSG + string(_ErrorBuffer) + "\n");
				}

    		}// Command node
    		// Loop node
    		else if (!xmlStrcmp(pRootNode->name, (const xmlChar *)LOOP_NODE_NAME))
    		{
    			unsigned short _LoopCount;
    			bool _Average;

    			// Get loop count attribute
    			xmlChar *_LoopCountAttribute = xmlGetProp(pRootNode, (const xmlChar *)COUNT_LOOP_ATTRIBUT_NAME);
    			if (!_LoopCountAttribute)
    				throw CMV2HostException(COUNT_ATTR_EXCEPTION_MSG);

    			// Convert _LoopCount
    			_LoopCount = strtol((char*)_LoopCountAttribute, NULL, 10);
    			// free memory
    			xmlFree(_LoopCountAttribute);

    			// Get average attribute
    			xmlChar *_AverageAttribute = xmlGetProp(pRootNode, (const xmlChar *)AVERAGE_LOOP_ATTRIBUT_NAME);
    			if (!_AverageAttribute)
    				throw CMV2HostException(PARSE_XML_FILE_AVER_ATTR_EXCEPTION_MSG);

    			// Convert _Average
    			_Average = strcmp((const char*)_AverageAttribute, "true") == 0;

    			// free memory
    			xmlFree(_AverageAttribute);

    			// Add loop start command to the buffer
    			rCommandsBuffer.push_back(CreateCommand(MV2_CMD_SET_LOOP_START, _LoopCount));

    			int _ResultsIndexOldSize = rResultsInfos.size();

    			// Fill command buffer
    			FillCommandsBufferFromXmlNodes (pRootNode->children, pXPathCtx, rCommandsBuffer, rResultsInfos);

    			// Check
    			if (rResultsInfos.size() > 0)
    			{
    				rResultsInfos[_ResultsIndexOldSize].Loop = _LoopCount;
    				rResultsInfos[_ResultsIndexOldSize].Average = _Average;
    				rResultsInfos[_ResultsIndexOldSize].NbCommands = rResultsInfos.size() - _ResultsIndexOldSize;
    			}
    			// Add loop end command to the buffer
    			rCommandsBuffer.push_back(CreateCommand(MV2_CMD_SET_LOOP_END, 0));

    		} // Loop node
    	}
    	// Update current node
    	pRootNode = pRootNode->next;
    }
	return rCommandsBuffer.size() - _OldSize;
} // FillCommandsBufferFromXmlNodes

// Compute response index
void CHostScript::ComputeResponseIndex (
										int				ResponseSize,		// Response size
										int				&rStatusIndex,		// Status index
										int				&rCrcIndex,			// CRC index
										int				&rStatusDescIndex,	// Status description index
										int				&rFirstDataIndex,	// First data index
										unsigned int	&rNbResults)		// Number of results
{
	// Expected response size must be superior or equal to the minimum response size
	// in order to compute index
	if (ResponseSize >= RESPONSE_MINIMUM_LENGTH)
	{
		rCrcIndex			= ResponseSize - 1;
		rStatusDescIndex	= rCrcIndex - 1;
		rStatusIndex		= rStatusDescIndex - 1;
		if (ResponseSize > RESPONSE_MINIMUM_LENGTH)
		{
			rFirstDataIndex = 1;
			rNbResults = rStatusIndex - rFirstDataIndex;
		}
		else
		{
			rFirstDataIndex = -1;
			rFirstDataIndex = 0;
		}
	} // Check response size
	// Error
	else
		throw CMV2HostException(COMPUTE_RESPONSE_INDEX_EXCEPTION_MSG);
} // ComputeResponseIndex

// Parse response buffer and fill results buffer according to ResultsInfos vector
void CHostScript::ParseResults (	tResult									*pResponseBuffer,	// Response buffer
									int										ResponseSize,		// Response size
									vector<tResultInfos>					ResultsInfos,		// Informations about results
									vector< vector<tResult> >				&rResults)			// Results
{
	// Make sure results buffer is empty
	rResults.clear();

	// Compute response index
	int _StatusIndex;
	int _StatusDescIndex;
	int _CrcIndex;
	int _FirstDataIndex;
	unsigned int _NbResults;
	ComputeResponseIndex(ResponseSize, _StatusIndex, _CrcIndex, _StatusDescIndex, _FirstDataIndex, _NbResults);

	// Get error code
	eError _Error = static_cast<eError>(pResponseBuffer[_StatusIndex]);

	// Handle error
	if (_Error != kNoError)
	{
		char _ErrorBuffer [10];
		snprintf (_ErrorBuffer, sizeof(_ErrorBuffer), "%d", pResponseBuffer[_StatusDescIndex]);
		throw CMV2HostException(MV2_EXCEPTION_MSG + gResponseErrorCodes[_Error] + ": " + string(_ErrorBuffer) + "\n");
	}

	// Find maximum output index
	int _MaxOutputIndex = FindMaxOutputIndex(ResultsInfos);

	// If there is no output index, there is no reason to continue
	if (_MaxOutputIndex < 0)
		return;

	// Allocate memory
	rResults.reserve(_MaxOutputIndex);

	// Allocate memory for all sub vectors
	for (int _i=0; _i<=_MaxOutputIndex; _i++)
	{
		vector<tResult> _Tmp;
		_Tmp.reserve(1);
		rResults.push_back(_Tmp);
	}

	unsigned int _i = 0;
	int _ResponseDataIndex = _FirstDataIndex;

	// Loop over results informations
	while (_i<ResultsInfos.size())
	{
		// Handle loop commands
		if (ResultsInfos[_i].Loop > 0)
		{
			// Initialize temp results
			vector< vector<tResult> > _ResultsTemp;
			// Allocate memory for _MaxOutputIndex elements
			_ResultsTemp.reserve(_MaxOutputIndex);
			// Allocate memory for all sub vectors
			for (int _n=0; _n<=_MaxOutputIndex; _n++)
			{
				vector<tResult> _Tmp;
				// Allocate memory at least for one element
				_Tmp.reserve(1);
				// Add vector
				_ResultsTemp.push_back(_Tmp);
			}

			// Handle all results inside the loop
			for (int _LoopCounter=0; _LoopCounter<ResultsInfos[_i].Loop; _LoopCounter++)
			{
				// For all commands inside the loop, get result in response buffer and store result only if necessary
				for (unsigned int _j=_i; _j<ResultsInfos[_i].NbCommands+_i; _j++)
				{
					// Store result only if necessary
					if(ResultsInfos[_j].OutputIndex >= 0)
						_ResultsTemp[ResultsInfos[_j].OutputIndex].push_back(pResponseBuffer[_ResponseDataIndex]);
					_ResponseDataIndex++;
				}
			} // Handle all results inside the loop

			// Update _Results
			for (unsigned int _k=0; _k<_ResultsTemp.size(); _k++)
			{
				// If results are averaged
				if (ResultsInfos[_i].Average)
				{
					rResults[_k].push_back(Average(_ResultsTemp[_k]));
				}
				else
				{
					for (unsigned int _l=0; _l<_ResultsTemp[_k].size(); _l++)
					{
						rResults[_k].push_back(_ResultsTemp[_k][_l]);
					}
				}
			} // End update _Results

			// Update _i according to commands inside the loop
			_i += ResultsInfos[_i].NbCommands;

		} // Handle loop commands
		else
		{
			// Store result only if necessary
			if(ResultsInfos[_i].OutputIndex >= 0)
				rResults[ResultsInfos[_i].OutputIndex].push_back(pResponseBuffer[_ResponseDataIndex]);
			_ResponseDataIndex++;
			_i++;
		}
	} // while (_i<ResultsInfos.size())
} // ParseResults

int CHostScript::FindMaxOutputIndex (std::vector<tResultInfos> ResultsInfos)
{
	int _MaxOutputIndex = -1;

	for (unsigned int _i=0; _i<ResultsInfos.size(); _i++)
	{
		if (ResultsInfos[_i].OutputIndex > _MaxOutputIndex)
			_MaxOutputIndex = ResultsInfos[_i].OutputIndex;
	}

	return _MaxOutputIndex;
} // FindMaxOutputIndex

// Compute Average
int CHostScript::Average (vector<tResult> Vector)
{
	int _Temp = 0;

	for (unsigned int _i=0; _i<Vector.size(); _i++)
		_Temp += Vector[_i];

	// Add size / 2 to avoid truncation error
	_Temp += Vector.size() / 2;
	// Compute average
	_Temp /= Vector.size();

	return _Temp;
} // Mean

// Convert results to CSV
string CHostScript::ConvertResultsToCSV (vector< vector<tResult> > Results)	// Results
{
	stringstream _Ss;
	unsigned int _LineIndex = 0;
	bool _HasMoreElements = false;
	
	// Create string stream from results
	while (true)
	{
		_HasMoreElements = false;
		
		// Loop over columns
		for (unsigned int _ColumnIndex=0; _ColumnIndex<Results.size(); _ColumnIndex++)
		{
		
			// Result available ?
			if (Results[_ColumnIndex].size() > _LineIndex)
			{
				// Add current result to the string stream
				_Ss << Results[_ColumnIndex][_LineIndex];
				// Check if there is one more element to add comma to the string stream 
				// according to CSV format
				if (_ColumnIndex < Results.size()-1)
					_Ss << ",";
				if (Results[_ColumnIndex].size() > (_LineIndex + 1))
					_HasMoreElements = true;
			}
			else
				_Ss << ",";
		} // Loop over columns
		// Add new line to the string stream
		_Ss << "\n";
		// Next line
		_LineIndex++;
		// Check if loop must continues
		if (!_HasMoreElements)
			break;
	}
	
	return _Ss.str();
} // ConvertResultsToCSV

// Convert vector to CSV format
string CHostScript::ConvertHeadingsToCSV (vector<string> Headings)	// Headings
{
	string _Str;

	for (unsigned int _i=0; _i<Headings.size(); _i++)
	{
		// Append current heading to the string
		_Str += Headings[_i];
		// Check if there is another value to add or not a comma
		if (_i < Headings.size()-1)
			_Str += ",";
	}

	return _Str;
} // ConvertHeadingsToCSV

} // namespace MV2Host
