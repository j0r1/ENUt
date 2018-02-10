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
#define SOCKETWAITER_ERRSTR_BADSOCKET						"One of the sockets has not been created yet"

namespace nut
{

SocketWaiter::SocketWaiter()
{
}

SocketWaiter::~SocketWaiter()
{
}

bool SocketWaiter::wait(int seconds, int microseconds)
{
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
		setErrorString(std::string(SOCKETWAITER_ERRSTR_SELECTERR) + getSocketErrorString());
		return false;
	}
	
	for (it = m_sockets.begin() ; it != m_sockets.end() ; it++)
	{
		if (FD_ISSET((*it)->getSocketDescriptor(), &fdset))
			(*it)->setDataAvailable(true);
	}
	
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

