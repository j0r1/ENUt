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

#include "nutconfig.h"
#include "socketwaiter.h"
#include "socket.h"
#if (defined(WIN32) || defined(_WIN32_WCE))
	#define NUTSOCKERR							INVALID_SOCKET
#else
	#include <sys/select.h>
	#include <errno.h>

	#define NUTSOCKERR							-1
#endif // !(WIN32 || _WIN32_WCE)

#define SOCKETWAITER_ERRSTR_SELECTERR						"Error in select call: "
#define SOCKETWAITER_ERRSTR_POLLERR						"Error in poll call: "
#define SOCKETWAITER_ERRSTR_BADSOCKET						"One of the sockets has not been created yet"

namespace nut
{

SocketWaiter::SocketWaiter()
{
}

SocketWaiter::SocketWaiter(const std::string &objName) : errut::ErrorBase(objName)
{
}

SocketWaiter::~SocketWaiter()
{
}

bool SocketWaiter::wait(int seconds, int microseconds)
{
#ifndef NUTCONFIG_SUPPORT_POLL
	fd_set fdset;
	std::list<Socket *>::const_iterator it;
	
	FD_ZERO(&fdset);
	for (it = m_sockets.begin() ; it != m_sockets.end() ; it++)
	{
		if ((*it)->getSocketDescriptor() == NUTSOCKERR)
		{
			setErrorString(SOCKETWAITER_ERRSTR_BADSOCKET);
			return false;
		}
		(*it)->setDataAvailable(false);
		FD_SET((*it)->getSocketDescriptor(), &fdset);
	}

	struct timeval tv;
	struct timeval *tv2;

	if (seconds >= 0 && microseconds >= 0)
	{
		tv2 = &tv;
		tv.tv_sec = seconds;
		tv.tv_usec = microseconds;
	}
	else
		tv2 = 0;
	
	if (select(FD_SETSIZE, &fdset, 0, 0, tv2) == NUTSOCKERR)
	{
		// TODO: for now, ignore signal interrupts
#if (defined(WIN32) || defined(_WIN32_WCE))
		if (WSAGetLastError() == WSAEINTR)
#else
		if (errno != EINTR) 
#endif // WIN32 || _WIN32_WCE
		{
			setErrorString(std::string(SOCKETWAITER_ERRSTR_SELECTERR) + getSocketErrorString());
			return false;
		}
	}
	
	for (it = m_sockets.begin() ; it != m_sockets.end() ; it++)
	{
		if (FD_ISSET((*it)->getSocketDescriptor(), &fdset))
			(*it)->setDataAvailable(true);
	}
#else // Use the 'poll()' function instead
	
	std::list<Socket *>::const_iterator it;

	m_pollInfo.resize(0);

	for (it = m_sockets.begin() ; it != m_sockets.end() ; it++)
	{
		if ((*it)->getSocketDescriptor() == NUTSOCKERR)
		{
			setErrorString(SOCKETWAITER_ERRSTR_BADSOCKET);
			return false;
		}
		(*it)->setDataAvailable(false);
		
		struct pollfd pollStruct;

		pollStruct.fd = (*it)->getSocketDescriptor();
		pollStruct.events = POLLIN;
		pollStruct.revents = 0;

		m_pollInfo.push_back(pollStruct);
	}
	
	int milliSeconds = -1;
	
	if (seconds >= 0 && microseconds >= 0)
		milliSeconds = microseconds/1000 + seconds*1000;

	if (poll(&(m_pollInfo[0]), m_pollInfo.size(), milliSeconds) < 0)
	{
		// TODO: for now, ignore signal interrupts
#if (defined(WIN32) || defined(_WIN32_WCE))
		if (WSAGetLastError() == WSAEINTR)
#else
		if (errno != EINTR) 
#endif // WIN32 || _WIN32_WCE
		{
			setErrorString(std::string(SOCKETWAITER_ERRSTR_POLLERR) + getSocketErrorString());
			return false;
		}
	}
	
	int counter = 0;
	
	for (it = m_sockets.begin() ; it != m_sockets.end() ; it++, counter++)
	{
		int revents = m_pollInfo[counter].revents;

		if (revents&(POLLIN|POLLHUP|POLLPRI))
			(*it)->setDataAvailable(true);
	}
#endif // NUTCONFIG_SUPPORT_POLL
	return true;
}
	
std::string SocketWaiter::getSocketErrorString()
{
#if (defined(WIN32) || defined(_WIN32_WCE))
	char str[32];

	_snprintf(str,32,"Winsock error: %d",WSAGetLastError());
	return std::string(str);
#else
	return std::string(strerror(errno));
#endif // WIN32 || _WIN32_WCE
}

} // end namespace

