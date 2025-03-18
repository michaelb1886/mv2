// Name:
//	CMV2HostException.h
//
// Purpose:
//	Custom MV2 exception object to be thrown.
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

#ifndef CMV2_HOST_EXCEPTION_H
#define CMV2_HOST_EXCEPTION_H

// Include files
#include <exception>
#include <string>

// Our namespace
namespace MV2Host
{
	class CMV2HostException: public std::exception
	{
		private :
			std::string		m_Message;		// Error message

		public :
			CMV2HostException(std::string const& rErrorMessage) throw ()
				: m_Message (rErrorMessage)
			{}

			virtual ~CMV2HostException() throw ()
			{}

			virtual const char* what() const throw ()
			{	return m_Message.c_str();	}
	};
} // namespace MV2Host
#endif // CMV2_HOST_EXCEPTION_H
