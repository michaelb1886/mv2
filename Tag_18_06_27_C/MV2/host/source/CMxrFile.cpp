// Name:
//	CMxrFile.cpp
//
// Purpose:
//	See CMxrFile.h
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
//
// Copyright (c) 2016 Metrolab Technology SA, Geneva,
//	Switzerland

// Include files
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <time.h>
#include <CMxrFile.h>

// Constants for MXR file
#define ROOT_NODE_NAME					BAD_CAST "MetrolabXmlRecord"
#define ROOT_NODE_ATTR_VER_NAME			BAD_CAST "ver"
#define ROOT_NODE_ATTR_VER_CONTENT		BAD_CAST "1.0"
#define SRC_NODE_NAME					BAD_CAST "src"
#define SRC_NODE_CONTENT				BAD_CAST "MV2 Host Software"
#define HEADER_NODE_NAME				BAD_CAST "header"
#define MEASUREMENT_NODE_NAME			BAD_CAST "meas"
#define DATE_NODE_NAME					BAD_CAST "datTim8601"
#define DESCR_NODE_NAME					BAD_CAST "descr"
#define DESCR_NODE_CONTENT				BAD_CAST "Results from MV2"
#define BODY_NODE_NAME					BAD_CAST "body"
#define BODY_ATTR_TYPE_NAME				BAD_CAST "type"
#define BODY_ATTR_TYPE_CONTENT			BAD_CAST "tMXR_BODY_MV2"
#define BODY_ATTR_VER_NAME				BAD_CAST "ver"
#define BODY_ATTR_VER_CONTENT			BAD_CAST "1.0"
#define DATASET_NODE_NAME				BAD_CAST "dataset"
#define DATASET_NODE_ATTR_TYPE_NAME		BAD_CAST "type"
#define DATASET_NODE_ATTR_TYPE_CONTENT	BAD_CAST "tMXR_DATASET_MV2_MEASUREMENT"
#define DATASET_NODE_ATTR_VER_NAME		BAD_CAST "ver"
#define DATASET_NODE_ATTR_VER_CONTENT	BAD_CAST "1.0"
#define HEADINGS_NODE_NAME				BAD_CAST "headings"

// XPath constants
#define DATASET_XPATH					BAD_CAST "/MetrolabXmlRecord/body/dataset"
#define HEADINGS_XPATH					BAD_CAST "/MetrolabXmlRecord/body/dataset/headings"

// Exceptions messages
#define NEW_XML_DOC_EXCEPTION_MSG		"CMxrFile: Unable to create new XML document.\n"
#define NEW_XML_NODE_EXCEPTION_MSG		"CMxrFile: Unable to create new XML node.\n"
#define ADD_XML_CHILD_EXCEPTION_MSG		"CMxrFile: Unable to add XML child.\n"
#define EVAL_XPATH_EXPR_EXCEPTION_MSG	"CMxrFile: Unable to evaluate XPath expression.\n"
#define SAVE_MXR_FILE_EXCEPTION_MSG		"CMxrFile: Unable to save MXR file.\n"
#define NEW_XPATH_CONTEXT_EXCEPTION_MSG	"CMxrFile: Unable to create new XPath context.\n"
#define CHECK_XML_NODE_EXCEPTION_MSG	"CMxrFile: Unable to check XML node.\n"

// Our namespace
namespace MV2Host
{

// Constructor
CMxrFile::CMxrFile(string Filename)
{
	// Update filename
	m_Filename = Filename;

	// Initialize libxml
	xmlInitParser();

	xmlNodePtr	_pRootNode		= NULL,
				_pBodyNode		= NULL,
				_pHeaderNode	= NULL,
				_pHeadingsNode	= NULL,
				_pDataSet		= NULL;

	char _TimeBuffer [80];
	struct tm * _TimeInfo;
	time_t _RawTime;

	// Get current time
	time (&_RawTime);

	// Convert _RawTime to local time
	_TimeInfo = localtime (&_RawTime);

	// Format _TimeInfo according to ISO-8601 format
	strftime (_TimeBuffer, 80, "%Y-%m-%dT%H:%M:%S", _TimeInfo);

	// Create new XML document
	if ( (m_pDoc = xmlNewDoc(BAD_CAST "1.0")) == NULL)
		throw CMV2HostException(NEW_XML_DOC_EXCEPTION_MSG);

	// Create root node with attributes
	if ((_pRootNode  = xmlNewNode(NULL, ROOT_NODE_NAME)) == NULL)
		throw CMV2HostException(NEW_XML_NODE_EXCEPTION_MSG);

	xmlNewProp(_pRootNode, ROOT_NODE_ATTR_VER_NAME, BAD_CAST "1.0");
	xmlDocSetRootElement(m_pDoc, _pRootNode);

	// Create header node
	if ((_pHeaderNode = xmlNewNode(NULL, (xmlChar*) HEADER_NODE_NAME)) == NULL)
		throw CMV2HostException(NEW_XML_NODE_EXCEPTION_MSG);

	// Add header node to the root node
	if (xmlAddChild(_pRootNode, _pHeaderNode) == NULL)
		throw CMV2HostException(ADD_XML_CHILD_EXCEPTION_MSG);

	// Add children to the header node
	if (xmlNewChild(_pHeaderNode, NULL, SRC_NODE_NAME, SRC_NODE_CONTENT) == NULL)
		throw CMV2HostException(NEW_XML_NODE_EXCEPTION_MSG);
	if (xmlNewChild(_pHeaderNode, NULL, DATE_NODE_NAME, BAD_CAST _TimeBuffer) == NULL)
		throw CMV2HostException(NEW_XML_NODE_EXCEPTION_MSG);
	if (xmlNewChild(_pHeaderNode, NULL, DESCR_NODE_NAME, DESCR_NODE_CONTENT) == NULL)
		throw CMV2HostException(NEW_XML_NODE_EXCEPTION_MSG);

	// Create body node with attributes
	if ((_pBodyNode = xmlNewNode(NULL, BODY_NODE_NAME)) == NULL)
		throw CMV2HostException(NEW_XML_NODE_EXCEPTION_MSG);
	xmlNewProp(_pBodyNode, BODY_ATTR_TYPE_NAME, BODY_ATTR_TYPE_CONTENT);
	xmlNewProp(_pBodyNode, BODY_ATTR_VER_NAME, BODY_ATTR_VER_CONTENT);

	// Create dataset node with attributes
	if ((_pDataSet = xmlNewNode(NULL, DATASET_NODE_NAME)) == NULL)
		throw CMV2HostException(NEW_XML_NODE_EXCEPTION_MSG);
	xmlNewProp(_pDataSet, DATASET_NODE_ATTR_TYPE_NAME, DATASET_NODE_ATTR_TYPE_CONTENT);
	xmlNewProp(_pDataSet, DATASET_NODE_ATTR_VER_NAME, DATASET_NODE_ATTR_VER_CONTENT);

	// Create headings node
	if ((_pHeadingsNode = xmlNewNode(NULL, HEADINGS_NODE_NAME)) == NULL)
		throw CMV2HostException(NEW_XML_NODE_EXCEPTION_MSG);

	// Set headings content to a default value
	xmlNodeSetContent(_pHeadingsNode, BAD_CAST "");

	// Add headings node to dataset node
	if (xmlAddChild(_pDataSet, _pHeadingsNode) == NULL)
		throw CMV2HostException(ADD_XML_CHILD_EXCEPTION_MSG);

	// Add dataset node to body node
	if (xmlAddChild(_pBodyNode, _pDataSet) == NULL)
		throw CMV2HostException(ADD_XML_CHILD_EXCEPTION_MSG);

	// Add body node to root node
	if (xmlAddChild(_pRootNode, _pBodyNode) == NULL)
		throw CMV2HostException(ADD_XML_CHILD_EXCEPTION_MSG);

	// Save and check file with indentation
	if (xmlSaveFormatFileEnc(Filename.c_str(), m_pDoc, "UTF-8", 1) < 0)
		throw CMV2HostException(SAVE_MXR_FILE_EXCEPTION_MSG);

	// Create XPath evaluation context
	xmlXPathObjectPtr _pXPathObj;
	m_pXPathCtx = xmlXPathNewContext(m_pDoc);
	if(m_pXPathCtx == NULL)
			throw CMV2HostException(NEW_XPATH_CONTEXT_EXCEPTION_MSG);

	// Evaluate DATASET_XPATH expression
	xmlChar* _pXpathExpr;
	_pXpathExpr = DATASET_XPATH;
	_pXPathObj = xmlXPathEvalExpression(_pXpathExpr, m_pXPathCtx);
	if(_pXPathObj == NULL)
		throw CMV2HostException(EVAL_XPATH_EXPR_EXCEPTION_MSG);

	// Check if there is at least one dataset node
	if (_pXPathObj->nodesetval->nodeNr < 1)
		throw CMV2HostException(CHECK_XML_NODE_EXCEPTION_MSG);

	// Evaluate HEADINGS_XPATH expression
	_pXpathExpr = HEADINGS_XPATH;
	m_pXPathObjHeadingsNode = xmlXPathEvalExpression(_pXpathExpr, m_pXPathCtx);
	if (m_pXPathObjHeadingsNode->nodesetval->nodeNr == 0)
		throw CMV2HostException(EVAL_XPATH_EXPR_EXCEPTION_MSG);

	// Set default content to headings node
	xmlNodeSetContent(m_pXPathObjHeadingsNode->nodesetval->nodeTab[0], BAD_CAST "");

	// Evaluate DATASET_XPATH expression for a future use
	m_pXPathObjDataSetNode = xmlXPathEvalExpression(DATASET_XPATH, m_pXPathCtx);

	// Activate indentation
	xmlKeepBlanksDefault(0);

	// Save file
	xmlSaveFormatFileEnc(Filename.c_str(), m_pDoc, "UTF-8", 1);
} // Constructor

// Destructor
CMxrFile::~CMxrFile()
{
	xmlXPathFreeContext(m_pXPathCtx);
	xmlXPathFreeObject(m_pXPathObjDataSetNode);
	xmlXPathFreeObject(m_pXPathObjHeadingsNode);
	xmlFreeDoc(m_pDoc);
	xmlCleanupParser();
} // Destructor

// Write results
void CMxrFile::WriteResults(
							string		Buffer,		// Buffer to write
							string		Headings)	// Headings
{
    xmlNodePtr	_pMeasNode = NULL;

    // Update headings
    xmlNodeSetContent(m_pXPathObjHeadingsNode->nodesetval->nodeTab[0], BAD_CAST Headings.c_str());

    // Create a new measurement node
    if ((_pMeasNode = xmlNewNode(NULL, MEASUREMENT_NODE_NAME)) == NULL)
    	throw CMV2HostException(NEW_XML_NODE_EXCEPTION_MSG);

    // Set content to the measurement node
    xmlNodeSetContent(_pMeasNode, BAD_CAST Buffer.c_str());

    // Add new measurement node
    if ((xmlAddChild(m_pXPathObjDataSetNode->nodesetval->nodeTab[0], _pMeasNode)) == NULL)
    	throw CMV2HostException(NEW_XML_NODE_EXCEPTION_MSG);

    // Save document
    if(xmlSaveFormatFileEnc(m_Filename.c_str(), m_pDoc, "UTF-8", 1) < 0)
    	throw CMV2HostException(SAVE_MXR_FILE_EXCEPTION_MSG);
} // Write results

} // namespace MV2Host
