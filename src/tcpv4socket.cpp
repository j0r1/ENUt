#include "nutconfig.h"
#include "tcpv4socket.h"
#include <string.h>

#define TCPV4SOCKET_ERRSTR_ALREADYCREATED					"Socket is already created"
#define TCPV4SOCKET_ERRSTR_NOTCREATED						"Socket is not yet created"
#define TCPV4SOCKET_ERRSTR_BADPROTOCOL						"Specified address is not an IPv4 address"
#define TCPV4SOCKET_ERRSTR_IOCTLERROR						"Error in 'ioctl' call: "
#define TCPV4SOCKET_ERRSTR_ALREADYCONNECTED					"Socket is already connected"
#define TCPV4SOCKET_ERRSTR_CANTCONNECT						"Error in 'connect' call: "
#define TCPV4SOCKET_ERRSTR_LISTENING						"Socket is in listening mode"
#define TCPV4SOCKET_ERRSTR_CANTLISTEN						"Error in 'listen' call: "
#define TCPV4SOCKET_ERRSTR_NOTLISTENING						"Socket is not in listening mode"
#define TCPV4SOCKET_ERRSTR_CANTACCEPT						"Error in 'accept' call: "
#define TCPV4SOCKET_ERRSTR_CANTWRITE						"Error in 'send' call: "
#define TCPV4SOCKET_ERRSTR_CANTREAD						"Error in 'recv' call: "
#define TCPV4SOCKET_ERRSTR_NOTCONNECTED						"No connection has been established yet"
#define TCPV4SOCKET_ERRSTR_CANTCREATESOCKET					"Error in 'socket' call: "
#define TCPV4SOCKET_ERRSTR_CANTBIND						"Error in 'bind' call: "
#define TCPV4SOCKET_ERRSTR_CANTGETLOCALPORT					"Error in 'getsockname' call: "
#define TCPV4SOCKET_ERRSTR_CANTSELECT						"Error in 'select' call: "
#if (defined(WIN32) || defined(_WIN32_WCE))
	#define NUTSOCKERR								INVALID_SOCKET
	#define NUTCLOSE(x)								closesocket(x)
	#define NUTSOCKLENTYPE							int
	#define NUTIOCTL								ioctlsocket
#else 
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/ioctl.h>
	#include <net/if.h>
	#include <string.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <errno.h>

	#define NUTSOCKERR								-1
	#define NUTCLOSE(x)								close(x)
	#define NUTSOCKLENTYPE							socklen_t
	#define NUTIOCTL								ioctl
#endif // WIN32 || _WIN32_WCE

namespace nut
{

TCPv4Socket::TCPv4Socket() : TCPSocket(IPv4)
{
	zeroAll();
}

TCPv4Socket::TCPv4Socket(const std::string &objName) : TCPSocket(objName, IPv4)
{
	zeroAll();
}

TCPv4Socket::~TCPv4Socket()
{
	destroy();
}

bool TCPv4Socket::create(uint16_t portNumber)
{
	return internalCreate(0, portNumber);
}

bool TCPv4Socket::create(const NetworkLayerAddress &bindAddress, uint16_t portNumber)
{
	if (bindAddress.getProtocol() != IPv4)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_BADPROTOCOL);
		return false;
	}
	
	const IPv4Address &ip = (const IPv4Address &)bindAddress;

	return internalCreate(ip.getAddress(), portNumber);
}

bool TCPv4Socket::destroy()
{
	if (m_sock == NUTSOCKERR)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_NOTCREATED);
		return false;
	}

	NUTCLOSE(m_sock);
	if (m_destIP)
		delete m_destIP;
	zeroAll();
	
	return true;
}

bool TCPv4Socket::setNonBlocking(bool f)
{
	if (m_sock == NUTSOCKERR)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	
	
#if (defined(WIN32) || defined(_WIN32_WCE))
	unsigned long flag = 0;
#else
	int flag = 0;
#endif // WIN32 || _WIN32_WCE

	if (f)
		flag = 1;
	
	if (NUTIOCTL(m_sock, FIONBIO, &flag) == NUTSOCKERR)
	{
		setErrorString(std::string(TCPV4SOCKET_ERRSTR_IOCTLERROR) + getSocketErrorString());
		return false;
	}
 
	return true;
}
	
bool TCPv4Socket::connect(const NetworkLayerAddress &hostAddress, uint16_t portNumber)
{
	if (m_sock == NUTSOCKERR)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	if (m_connected)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_ALREADYCONNECTED);
		return false;
	}

	if (m_listening)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_LISTENING);
		return false;
	}

	if (hostAddress.getProtocol() != IPv4)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_BADPROTOCOL);
		return false;
	}

	const IPv4Address &ip = (const IPv4Address &)hostAddress;	
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ip.getAddress());
	addr.sin_port = htons(portNumber);

	if (::connect(m_sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) != 0)
	{
		setErrorString(std::string(TCPV4SOCKET_ERRSTR_CANTCONNECT) + getSocketErrorString());
		return false;
	}

	m_destIP = (IPv4Address *)(ip.createCopy());
	m_destPort = portNumber;
	m_connected = true;
	return true;
}

bool TCPv4Socket::listen(int backlog)
{
	if (m_sock == NUTSOCKERR)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	if (m_connected)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_ALREADYCONNECTED);
		return false;
	}

	if (m_listening)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_LISTENING);
		return false;
	}

	if (::listen(m_sock, backlog) != 0)
	{
		setErrorString(std::string(TCPV4SOCKET_ERRSTR_CANTLISTEN) + getSocketErrorString());
		return false;
	}

	m_listening = true;
	return true;
}

bool TCPv4Socket::accept(TCPSocket **newSocket)
{
	if (m_sock == NUTSOCKERR)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	if (!m_listening)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_NOTLISTENING);
		return false;
	}

#if (defined(WIN32) || defined(_WIN32_WCE))
	SOCKET newSock;
	int addrLen = sizeof(struct sockaddr_in);
#else
	int newSock;
	socklen_t addrLen = sizeof(struct sockaddr_in);
#endif // WIN32 || _WIN32_WCE
	struct sockaddr_in addr;

	memset(&addr, 0, addrLen);
	if ((newSock = ::accept(m_sock, (struct sockaddr *)&addr, &addrLen)) == NUTSOCKERR)
	{
		setErrorString(std::string(TCPV4SOCKET_ERRSTR_CANTACCEPT) + getSocketErrorString());
		return false;
	}

	// Ok, connection accepted
	
	*newSocket = new TCPv4Socket(newSock, m_localPort, ntohl(addr.sin_addr.s_addr), ntohs(addr.sin_port));
	return true;
}

bool TCPv4Socket::write(const void *data, size_t &length)
{
	if (m_sock == NUTSOCKERR)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	if (!m_connected)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_NOTCONNECTED);
		return false;
	}

#if (defined(WIN32) || defined(_WIN32_WCE))
	int x = (int)length;
	int y;
	int flags = 0;
#else
	size_t x = length;
	ssize_t y;
	int flags = MSG_NOSIGNAL;
#endif // WIN32 || _WIN32_WCE

	y = send(m_sock, (const char *)data, x, flags);
	if (y == NUTSOCKERR)
	{
		setErrorString(std::string(TCPV4SOCKET_ERRSTR_CANTWRITE) + getSocketErrorString());
		return false;
	}
	length = (size_t)y;
	
	return true;
}

bool TCPv4Socket::getAvailableDataLength(size_t &length)
{
	if (m_sock == NUTSOCKERR)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	if (!m_connected)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_NOTCONNECTED);
		return false;
	}

#if (defined(WIN32) || defined(_WIN32_WCE))
	unsigned long num = 0;
#else
	size_t num = 0;
#endif // WIN32 || _WIN32_WCE

	if (NUTIOCTL(m_sock, FIONREAD, &num) != 0)
	{
		setErrorString(std::string(TCPV4SOCKET_ERRSTR_IOCTLERROR) + getSocketErrorString());
		return false;
	}

	length = (size_t)num;
	return true;
}

bool TCPv4Socket::read(void *buffer, size_t &bufferSize)
{
	if (m_sock == NUTSOCKERR)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	if (!m_connected)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_NOTCONNECTED);
		return false;
	}
#if (defined(WIN32) || defined(_WIN32_WCE))
	int x = (int)bufferSize;
	int y;
#else
	size_t x = bufferSize;
	ssize_t y;
#endif // WIN32 || _WIN32_WCE

	if ((y = recv(m_sock, (char *)buffer, x, 0)) == -1)
	{
		setErrorString(std::string(TCPV4SOCKET_ERRSTR_CANTREAD) + getSocketErrorString());
		return false;
	}
	
	bufferSize = (size_t)y;
	return true;
}

bool TCPv4Socket::internalCreate(uint32_t ip, uint16_t port)
{
	if (m_sock != NUTSOCKERR)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_ALREADYCREATED);
		return false;
	}
	
	zeroAll();

#if (defined(WIN32) || defined(_WIN32_WCE))
	SOCKET s;
#else
	int s;
#endif // WIN32 || _WIN32_WCE

	if ((s = socket(PF_INET, SOCK_STREAM, 0)) == NUTSOCKERR)
	{
		setErrorString(std::string(TCPV4SOCKET_ERRSTR_CANTCREATESOCKET) + getSocketErrorString());
		return false;
	}
	
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ip);
	addr.sin_port = htons(port);

	if (bind(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) != 0)
	{
		NUTCLOSE(s);
		setErrorString(std::string(TCPV4SOCKET_ERRSTR_CANTBIND) + getSocketErrorString());
		return false;
	}

#if (defined(WIN32) || defined(_WIN32_WCE))
	int addrLen = sizeof(struct sockaddr_in);
#else
	socklen_t addrLen = sizeof(struct sockaddr_in);
#endif // WIN32 || _WIN32_WCE
	memset(&addr, 0, addrLen);
	
	if (getsockname(s, (struct sockaddr *)&addr, &addrLen) != 0)
	{
		NUTCLOSE(s);
		setErrorString(std::string(TCPV4SOCKET_ERRSTR_CANTGETLOCALPORT) + getSocketErrorString());
		return false;
	}

	m_sock = s;
	m_localPort = ntohs(addr.sin_port);

	return true;
}

void TCPv4Socket::zeroAll()
{
	m_sock = NUTSOCKERR;
	m_connected = false;
	m_listening = false;
	m_localPort = 0;
	m_destPort = 0;
	m_destIP = 0;
}

#if (defined(WIN32) || defined(_WIN32_WCE))
TCPv4Socket::TCPv4Socket(SOCKET sock, uint16_t localPort, uint32_t destIP, uint16_t destPort) : TCPSocket(IPv4)
#else
TCPv4Socket::TCPv4Socket(int sock, uint16_t localPort, uint32_t destIP, uint16_t destPort) : TCPSocket(IPv4)
#endif // WIN32 || _WIN32_WCE
{
	zeroAll();
	m_sock = sock;
	m_connected = true;
	m_destIP = new IPv4Address(destIP);
	m_destPort = destPort;
	m_localPort = localPort;
}

bool TCPv4Socket::waitForData(int seconds, int microSeconds)
{
	if (m_sock == NUTSOCKERR)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

/*	if (!m_connected)
	{
		setErrorString(TCPV4SOCKET_ERRSTR_NOTCONNECTED);
		return false;
	}
*/
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

	if (status == NUTSOCKERR)
	{
		setErrorString(std::string(TCPV4SOCKET_ERRSTR_CANTSELECT) + getSocketErrorString());
		return false;
	}
	
	return true;
}

std::string TCPv4Socket::getSocketErrorString()
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

