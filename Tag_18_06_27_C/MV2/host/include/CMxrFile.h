// Name:
//	CMxrFile.h
//
// Purpose:
//	Handle MXR file
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

#ifndef CMXR_FILE_H
#define CMXR_FILE_H

// Include files
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <string>
#include <CMV2HostException.h>

using namespace std;

// Our namespace
namespace MV2Host
{
	class CMxrFile
	{
	public:

		// Constructor
		CMxrFile (
							string		Filename);		// Filename
		// Destructor
		~CMxrFile ();

		// Write results
		void WriteResults (
							string		Buffer,			// Buffer to write
							string		Headings);		// Headings

	private:
		string				m_Filename;					// Filename
		xmlDocPtr			m_pDoc;						// Pointer to the XML document
		xmlXPathContextPtr	m_pXPathCtx;				// Pointer to the XPath context
		xmlXPathObjectPtr	m_pXPathObjDataSetNode;		// Pointer to the data set XPath object
		xmlXPathObjectPtr	m_pXPathObjHeadingsNode;	// Pointer to the headings XPath object
	}; // CMxrFile

} // namespace MV2Host
#endif // CMXR_FILE_H
