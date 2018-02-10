/*
    
  This file is a part of ENUt, a library containing network
  programming utilities.
  
  Copyright (C) 2006-2012  Hasselt University - Expertise Centre for
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
 * \file socketwaiter.h
 */

#ifndef NUT_SOCKETWAITER_H

#define NUT_SOCKETWAITER_H

#include "nutconfig.h"
#include <errut/errorbase.h>
#include <list>

#ifdef NUTCONFIG_SUPPORT_POLL
#include <sys/poll.h>
#include <vector>
#endif // NUTCONFIG_SUPPORT_POLL

namespace nut
{

class Socket;
	
/** Allows you to wait for data to arrive on one or more sockets. 
 *  Allows you to wait for data to arrive on one or more sockets. This is
 *  basically a wrapper class for using the 'select' function. */
class ENUT_IMPORTEXPORT SocketWaiter : public errut::ErrorBase
{
public:
	SocketWaiter();
	SocketWaiter(const std::string &objName);
	~SocketWaiter();

	/** Adds a socket to the list of sockets which should be monitored. */
	void addSocket(Socket &s)								{ m_sockets.push_back(&s); }

	/** Remove a socket from the list of sockets which should be monitored. */
	void removeSocket(Socket &s) 								{ m_sockets.remove(&s); }

	/** Clears the list of sockets which should be monitored. */
	void clear()										{ m_sockets.clear(); }

	/** Waits for data to become available on the sockets which are being monitored.
	 *  Waits for data to become available on the sockets which are being monitored.
	 *  If one of the sockets has available data (or an incoming connection), the 
	 *  corresponding Socket::isDataAvailable function will then return true.
	 *  The maximum amount of time to wait is specified by \c seconds and 
	 *  \c microSeconds.
	 */
	bool wait(int seconds = -1, int microSeconds = -1);
private:
	// make sure we can't copy a socket waiter
	SocketWaiter(const SocketWaiter &s) : errut::ErrorBase(s)				{ }
	SocketWaiter &operator=(const SocketWaiter &s)						{ return *this; }
	
	std::string getSocketErrorString();
	std::list<Socket *> m_sockets;
#ifdef NUTCONFIG_SUPPORT_POLL
	std::vector<struct pollfd> m_pollInfo;
#endif // NUTCONFIG_SUPPORT_POLL
};
	
} // end namespace

#endif // NUT_SOCKETWAITER_H

