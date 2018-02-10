#include "nutconfig.h"

#ifdef NUTCONFIG_SUPPORT_IPV6

#include "tcpv6socket.h"
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

#define TCPV6SOCKET_ERRSTR_ALREADYCREATED					"Socket is already created"
#define TCPV6SOCKET_ERRSTR_NOTCREATED						"Socket is not yet created"
#define TCPV6SOCKET_ERRSTR_BADPROTOCOL						"Specified address is not an IPv6 address"
#define TCPV6SOCKET_ERRSTR_IOCTLERROR						"Error in 'ioctl' call: "
#define TCPV6SOCKET_ERRSTR_ALREADYCONNECTED					"Socket is already connected"
#define TCPV6SOCKET_ERRSTR_CANTCONNECT						"Error in 'connect' call: "
#define TCPV6SOCKET_ERRSTR_LISTENING						"Socket is in listening mode"
#define TCPV6SOCKET_ERRSTR_CANTLISTEN						"Error in 'listen' call: "
#define TCPV6SOCKET_ERRSTR_NOTLISTENING						"Socket is not in listening mode"
#define TCPV6SOCKET_ERRSTR_CANTACCEPT						"Error in 'accept' call: "
#define TCPV6SOCKET_ERRSTR_CANTWRITE						"Error in 'send' call: "
#define TCPV6SOCKET_ERRSTR_CANTREAD						"Error in 'recv' call: "
#define TCPV6SOCKET_ERRSTR_NOTCONNECTED						"No connection has been established yet"
#define TCPV6SOCKET_ERRSTR_CANTCREATESOCKET					"Error in 'socket' call: "
#define TCPV6SOCKET_ERRSTR_CANTBIND						"Error in 'bind' call: "
#define TCPV6SOCKET_ERRSTR_CANTGETLOCALPORT					"Error in 'getsockname' call: "
#define TCPV6SOCKET_ERRSTR_CANTSELECT						"Error in 'select' call: "

namespace nut
{

TCPv6Socket::TCPv6Socket() : TCPSocket(IPv6)
{
	zeroAll();
}

TCPv6Socket::TCPv6Socket(const std::string &objName) : TCPSocket(objName, IPv6)
{
	zeroAll();
}

TCPv6Socket::~TCPv6Socket()
{
	destroy();
}

bool TCPv6Socket::create(uint16_t portNumber)
{
	return internalCreate(in6addr_any, portNumber);
}

bool TCPv6Socket::create(const NetworkLayerAddress &bindAddress, uint16_t portNumber)
{
	if (bindAddress.getProtocol() != IPv6)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_BADPROTOCOL);
		return false;
	}
	
	const IPv6Address &ip = (const IPv6Address &)bindAddress;

	return internalCreate(ip.getAddress(), portNumber);
}

bool TCPv6Socket::destroy()
{
	if (m_sock == -1)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_NOTCREATED);
		return false;
	}

	close(m_sock);
	if (m_destIP)
		delete m_destIP;
	zeroAll();
	
	return true;
}

bool TCPv6Socket::setNonBlocking(bool f)
{
	if (m_sock == -1)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	
	
	int flag = 0;

	if (f)
		flag = 1;
	
	if (ioctl(m_sock, FIONBIO, &flag) == -1)
	{
		setErrorString(std::string(TCPV6SOCKET_ERRSTR_IOCTLERROR) + std::string(strerror(errno)));
		return false;
	}
 
	return true;
}
	
bool TCPv6Socket::connect(const NetworkLayerAddress &hostAddress, uint16_t portNumber)
{
	if (m_sock == -1)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	if (m_connected)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_ALREADYCONNECTED);
		return false;
	}

	if (m_listening)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_LISTENING);
		return false;
	}

	if (hostAddress.getProtocol() != IPv6)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_BADPROTOCOL);
		return false;
	}

	const IPv6Address &ip = (const IPv6Address &)hostAddress;	
	struct sockaddr_in6 addr;

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin6_family = AF_INET6;
	addr.sin6_addr = ip.getAddress();
	addr.sin6_port = htons(portNumber);

	if (::connect(m_sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in6)) != 0)
	{
		setErrorString(std::string(TCPV6SOCKET_ERRSTR_CANTCONNECT) + std::string(strerror(errno)));
		return false;
	}

	m_destIP = (IPv6Address *)(ip.createCopy());
	m_destPort = portNumber;
	m_connected = true;
	return true;
}

bool TCPv6Socket::listen(int backlog)
{
	if (m_sock == -1)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	if (m_connected)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_ALREADYCONNECTED);
		return false;
	}

	if (m_listening)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_LISTENING);
		return false;
	}

	if (::listen(m_sock, backlog) != 0)
	{
		setErrorString(std::string(TCPV6SOCKET_ERRSTR_CANTLISTEN) + std::string(strerror(errno)));
		return false;
	}

	m_listening = true;
	return true;
}

bool TCPv6Socket::accept(TCPSocket **newSocket)
{
	if (m_sock == -1)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	if (!m_listening)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_NOTLISTENING);
		return false;
	}

	int newSock;
	struct sockaddr_in6 addr;
	socklen_t addrLen = sizeof(struct sockaddr_in6);

	memset(&addr, 0, addrLen);
	if ((newSock = ::accept(m_sock, (struct sockaddr *)&addr, &addrLen)) == -1)
	{
		setErrorString(std::string(TCPV6SOCKET_ERRSTR_CANTACCEPT) + std::string(strerror(errno)));
		return false;
	}

	// Ok, connection accepted
	
	*newSocket = new TCPv6Socket(newSock, m_localPort, addr.sin6_addr, ntohs(addr.sin6_port));
	return true;
}

bool TCPv6Socket::write(const void *data, size_t &length)
{
	if (m_sock == -1)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	if (!m_connected)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_NOTCONNECTED);
		return false;
	}

	size_t x;
	ssize_t y;

	x = length;
	y = send(m_sock, data, x, MSG_NOSIGNAL);
	if (y == -1)
	{
		setErrorString(std::string(TCPV6SOCKET_ERRSTR_CANTWRITE) + std::string(strerror(errno)));
		return false;
	}
	length = (size_t)y;
	
	return true;
}

bool TCPv6Socket::getAvailableDataLength(size_t &length)
{
	if (m_sock == -1)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	if (!m_connected)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_NOTCONNECTED);
		return false;
	}

	size_t num;

	if (ioctl(m_sock, FIONREAD, &num) != 0)
	{
		setErrorString(std::string(TCPV6SOCKET_ERRSTR_IOCTLERROR) + std::string(strerror(errno)));
		return false;
	}

	length = num;
	return true;
}

bool TCPv6Socket::read(void *buffer, size_t &bufferSize)
{
	if (m_sock == -1)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	if (!m_connected)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_NOTCONNECTED);
		return false;
	}

	size_t x = bufferSize;
	ssize_t y;

	if ((y = recv(m_sock, buffer, x, 0)) == -1)
	{
		setErrorString(std::string(TCPV6SOCKET_ERRSTR_CANTREAD) + std::string(strerror(errno)));
		return false;
	}
	
	bufferSize = (size_t)y;
	return true;
}

bool TCPv6Socket::internalCreate(in6_addr ip, uint16_t port)
{
	if (m_sock != -1)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_ALREADYCREATED);
		return false;
	}
	
	zeroAll();
	
	int s;

	if ((s = socket(PF_INET6, SOCK_STREAM, 0)) == -1)
	{
		setErrorString(std::string(TCPV6SOCKET_ERRSTR_CANTCREATESOCKET) + std::string(strerror(errno)));
		return false;
	}
	
	struct sockaddr_in6 addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	
	addr.sin6_family = AF_INET6;
	addr.sin6_addr = ip;
	addr.sin6_port = htons(port);

	if (bind(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_in6)) != 0)
	{
		close(s);
		setErrorString(std::string(TCPV6SOCKET_ERRSTR_CANTBIND) + std::string(strerror(errno)));
		return false;
	}

	socklen_t addrLen = sizeof(struct sockaddr_in6);
	memset(&addr, 0, addrLen);
	
	if (getsockname(s, (struct sockaddr *)&addr, &addrLen) != 0)
	{
		close(s);
		setErrorString(std::string(TCPV6SOCKET_ERRSTR_CANTGETLOCALPORT) + std::string(strerror(errno)));
		return false;
	}

	m_sock = s;
	m_localPort = ntohs(addr.sin6_port);

	return true;
}

void TCPv6Socket::zeroAll()
{
	m_sock = -1;
	m_connected = false;
	m_listening = false;
	m_localPort = 0;
	m_destPort = 0;
	m_destIP = 0;
}

TCPv6Socket::TCPv6Socket(int sock, uint16_t localPort, in6_addr destIP, uint16_t destPort) : TCPSocket(IPv6)
{
	zeroAll();
	m_sock = sock;
	m_connected = true;
	m_destIP = new IPv6Address(destIP);
	m_destPort = destPort;
	m_localPort = localPort;
}

bool TCPv6Socket::waitForData(int seconds, int microSeconds)
{
	if (m_sock == -1)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	/*if (!m_connected)
	{
		setErrorString(TCPV6SOCKET_ERRSTR_NOTCONNECTED);
		return false;
	}*/

	fd_set fdset;
	int status;
	
	FD_ZERO(&fdset);
	FD_SET(m_sock, &fdset);

	if (seconds >= 0 && microSeconds >= 0)
	{
		struct timeval tv;

		tv.tv_sec = seconds;
		tv.tv_usec = microSeconds;
		
		status = select(FD_SETSIZE, &fdset, 0, 0, &tv);
	}
	else
		status = select(FD_SETSIZE, &fdset, 0, 0, 0);

	if (status == -1)
	{
		setErrorString(std::string(TCPV6SOCKET_ERRSTR_CANTSELECT) + std::string(strerror(errno)));
		return false;
	}
	
	return true;
}

} // end namespace

#endif // NUTCONFIG_SUPPORT_IPV6

