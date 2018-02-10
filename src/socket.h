#ifndef NUT_SOCKET_H

#define NUT_SOCKET_H

#include "nutconfig.h"
#include "nuttypes.h"
#include "errorbase.h"
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

class Socket : public ErrorBase
{
protected:
	Socket()											{ m_dataAvailable = false; }
	Socket(const std::string &objName) : ErrorBase(objName)						{ }
public:
	~Socket()											{ }

	bool setSocketOption(int level, int optname, const void *optval, socklen_t optlen);
	bool getSocketOption(int level, int optname, void *optval, socklen_t *optlen);
	
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
	
inline bool Socket::setSocketOption(int level, int optname, const void *optval, socklen_t optlen)
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

inline bool Socket::getSocketOption(int level, int optname, void *optval, socklen_t *optlen)
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

} // end namespace
	
#endif // NUT_SOCKET_H

