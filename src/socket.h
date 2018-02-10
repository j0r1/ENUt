/*
    
  This file is a part of ENUt, a library containing network
  programming utilities.
  
  Copyright (C) 2006-2008  Hasselt University - Expertise Centre for
                      Digital Media (EDM) (http://www.edm.uhasselt.be)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  
  USA

*/

/**
 * \file socket.h
 */

#ifndef NUT_SOCKET_H

#define NUT_SOCKET_H

#include "nutconfig.h"
#include "nuttypes.h"
#include <errut/errorbase.h>
#include <stdio.h>
#if !(defined(WIN32) || defined(_WIN32_WCE))
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/ioctl.h>
	#include <net/if.h>
	#include <string.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <errno.h>
#endif // ! (WIN32 || _WIN32_WCE)

namespace nut
{

/** Base class for sockets. */
class Socket : public errut::ErrorBase
{
protected:
	/** Creates an unnamed socket. */
	Socket()											{ m_dataAvailable = false; }

	/** Creates a socket with object name \c objName. */
	Socket(const std::string &objName) : errut::ErrorBase(objName)					{ }
public:
	~Socket()											{ }

	/** Sets a specific socket option.
	 *  Sets a specific socket option. This function is basically a wrapper for the
	 *  \c setsockopt function. */
	bool setSocketOption(int level, int optname, const void *optval, socklen_t optlen);

	/** Retrieves a specific socket option.
	 *  Retrieves a specific socket option. This function is basically a wrapper for
	 *  the \c getsockopt function. */
	bool getSocketOption(int level, int optname, void *optval, socklen_t *optlen);
	
	/** Wait for incoming data or for an incoming connection request.
	 *  Wait for incoming data or for an incoming connection request.
	 *  The maximum time to wait is specified by \c seconds and
	 *  \c microSeconds. This function is equivalent with using a
	 *  SocketWaiter instance, adding this socket and calling the
	 *  SocketWaiter::wait function. As a result, the isDataAvailable
	 *  function can be used after calling this function.
	 */
	bool waitForData(int seconds = -1, int microSeconds = -1);

	/** When the socket was used by a SocketWaiter instance, this flag indicates if
	 *  data is available after the SocketWaiter::wait function was called (this function
	 *  is not used in a socket's getAvailableDataLength function). 
	 */
	bool isDataAvailable() const									{ return m_dataAvailable; }
protected:
#if (defined(WIN32) || defined(_WIN32_WCE))
	virtual SOCKET getSocketDescriptor() = 0;
#else
	virtual int getSocketDescriptor() = 0;
#endif // WIN32 || _WIN32_WCE
private:
	void setDataAvailable(bool f)									{ m_dataAvailable = f; }

	bool m_dataAvailable;
	
	friend class SocketWaiter;
};
	
} // end namespace
	
#endif // NUT_SOCKET_H

