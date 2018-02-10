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
#include "socket.h"
#include "socketwaiter.h"

namespace nut
{

bool Socket::setSocketOption(int level, int optname, const void *optval, socklen_t optlen)
{
#if (defined(WIN32) || defined(_WIN32_WCE))
	int status = setsockopt(getSocketDescriptor(), level, optname, (const char *)optval, optlen);
#else
	int status = setsockopt(getSocketDescriptor(), level, optname, optval, optlen);
#endif // WIN32 || _WIN32_WCE
	
	if (status != 0)
	{
#if (defined(WIN32) || defined(_WIN32_WCE))
		char str[32];

		_snprintf(str,32,"Winsock error: %d",WSAGetLastError());
		setErrorString(std::string("Couldn't set socket option: ") + std::string(str));
#else
		setErrorString(std::string("Couldn't set socket option: ") + std::string(strerror(errno)));
#endif // WIN32 || _WIN32_WCE
		return false;
	}
	return true;
}

bool Socket::getSocketOption(int level, int optname, void *optval, socklen_t *optlen)
{
#if (defined(WIN32) || defined(_WIN32_WCE))
	int status = getsockopt(getSocketDescriptor(), level, optname, (char *)optval, optlen);
#else
	int status = getsockopt(getSocketDescriptor(), level, optname, optval, optlen);
#endif // WIN32 || _WIN32_WCE
	if (status != 0)
	{
#if (defined(WIN32) || defined(_WIN32_WCE))
		char str[32];

		_snprintf(str,32,"Winsock error: %d",WSAGetLastError());
		setErrorString(std::string("Couldn't get socket option: ") + std::string(str));
#else
		setErrorString(std::string("Couldn't get socket option: ") + std::string(strerror(errno)));
#endif // WIN32 || _WIN32_WCE
		return false;
	}
	return true;
}

bool Socket::waitForData(int seconds, int microSeconds)
{
	SocketWaiter waiter;

	waiter.addSocket(*this);
	if (!waiter.wait(seconds, microSeconds))
	{
		setErrorString(waiter.getErrorString());
		return false;
	}

	return true;
}

} // end namespace

