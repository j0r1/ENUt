#include "nutconfig.h"
#include "udpv4socket.h"
#include "ipv4address.h"
#include <string.h>

#define UDPV4SOCKET_ERRSTR_ALREADYCREATED					"Socket is already created"
#define UDPV4SOCKET_ERRSTR_NOTCREATED						"Socket is not yet created"
#define UDPV4SOCKET_ERRSTR_BADPROTOCOL						"Specified address is not an IPv4 address"
#define UDPV4SOCKET_ERRSTR_IOCTLERROR						"Error in 'ioctl' call: "
#define UDPV4SOCKET_ERRSTR_CANTWRITE						"Error in 'sendto' call: "
#define UDPV4SOCKET_ERRSTR_CANTREAD						"Error in 'recvmsg' or WSARecvMsg call: "
#define UDPV4SOCKET_ERRSTR_CANTCREATESOCKET					"Error in 'socket' call: "
#define UDPV4SOCKET_ERRSTR_CANTBIND						"Error in 'bind' call: "
#define UDPV4SOCKET_ERRSTR_CANTGETLOCALPORT					"Error in 'getsockname' call: "
#define UDPV4SOCKET_ERRSTR_NOTMCASTADDR						"Specified address is not a multicast address"
#define UDPV4SOCKET_ERRSTR_SETSOCKOPTERROR					"Error in 'setsockopt' call: "
#define UDPV4SOCKET_ERRSTR_CANTSELECT						"Error in 'select' call: "
#define UDPV4SOCKET_ERRSTR_CANTENABLEPKTINFO					"Can't enable IP_PKTINFO: "
#define UDPV4SOCKET_ERRSTR_CANTGETRECVMSG					"Can't get WSARecvMsg function pointer"
#define UDPV4SOCKET_ERRSTR_DESTADDRNOTSUPPORTED					"Obtaining destination IP of incoming packets is not supported"

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

UDPv4Socket::UDPv4Socket() : UDPSocket(IPv4)
{
	zeroAll();
}

UDPv4Socket::UDPv4Socket(const std::string &objName) : UDPSocket(objName, IPv4)
{
	zeroAll();
}

UDPv4Socket::~UDPv4Socket()
{
	destroy();
}

bool UDPv4Socket::create(uint16_t portNumber, bool obtainDestination)
{
	return internalCreate(0, portNumber, obtainDestination);
}

bool UDPv4Socket::create(NetworkLayerAddress &bindAddress, uint16_t portNumber, bool obtainDestination)
{
	if (bindAddress.getProtocol() != IPv4)
	{
		setErrorString(UDPV4SOCKET_ERRSTR_BADPROTOCOL);
		return false;
	}
	
	const IPv4Address &ip = (const IPv4Address &)bindAddress;

	return internalCreate(ip.getAddress(), portNumber, obtainDestination);
}

bool UDPv4Socket::destroy()
{
	if (m_sock == NUTSOCKERR)
	{
		setErrorString(UDPV4SOCKET_ERRSTR_NOTCREATED);
		return false;
	}

	NUTCLOSE(m_sock);
	zeroAll();
	
	return true;
}
	
bool UDPv4Socket::setNonBlocking(bool f)
{
	if (m_sock == NUTSOCKERR)
	{
		setErrorString(UDPV4SOCKET_ERRSTR_NOTCREATED);
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
		setErrorString(std::string(UDPV4SOCKET_ERRSTR_IOCTLERROR) + getSocketErrorString());
		return false;
	}
	
	if (f)
		m_isBlocking = false;
	else
		m_isBlocking = true;

 	return true;
}

#define UDPV4SOCKET_IS_MCASTADDR(x)							(((x)&0xF0000000) == 0xE0000000)
#define UDPV4SOCKET_MCASTMEMBERSHIP(socket,type,mcastip,status)				{\
												struct ip_mreq mreq;\
												\
												mreq.imr_multiaddr.s_addr = htonl(mcastip);\
												mreq.imr_interface.s_addr = htonl(m_bindIP);\
												status = setsockopt(socket,IPPROTO_IP,type,(const char *)&mreq,sizeof(struct ip_mreq));\
											}
	
bool UDPv4Socket::joinMulticastGroup(const NetworkLayerAddress &groupAddress)
{
	if (m_sock == NUTSOCKERR)
	{
		setErrorString(UDPV4SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	if (groupAddress.getProtocol() != IPv4)
	{
		setErrorString(UDPV4SOCKET_ERRSTR_BADPROTOCOL);
		return false;
	}

	const IPv4Address &ip = (const IPv4Address &)groupAddress;
	int status;

	if (!UDPV4SOCKET_IS_MCASTADDR(ip.getAddress()))
	{
		setErrorString(UDPV4SOCKET_ERRSTR_NOTMCASTADDR);
		return false;
	}

	UDPV4SOCKET_MCASTMEMBERSHIP(m_sock, IP_ADD_MEMBERSHIP, ip.getAddress(), status);
	if (status != 0)
	{
		setErrorString(std::string(UDPV4SOCKET_ERRSTR_SETSOCKOPTERROR) + getSocketErrorString());
		return false;
	}
	
	return true;
}

bool UDPv4Socket::leaveMulticastGroup(const NetworkLayerAddress &groupAddress)
{
	if (m_sock == NUTSOCKERR)
	{
		setErrorString(UDPV4SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	if (groupAddress.getProtocol() != IPv4)
	{
		setErrorString(UDPV4SOCKET_ERRSTR_BADPROTOCOL);
		return false;
	}

	const IPv4Address &ip = (const IPv4Address &)groupAddress;
	int status;

	if (!UDPV4SOCKET_IS_MCASTADDR(ip.getAddress()))
	{
		setErrorString(UDPV4SOCKET_ERRSTR_NOTMCASTADDR);
		return false;
	}

	UDPV4SOCKET_MCASTMEMBERSHIP(m_sock, IP_DROP_MEMBERSHIP, ip.getAddress(), status);
	if (status != 0)
	{
		setErrorString(std::string(UDPV4SOCKET_ERRSTR_SETSOCKOPTERROR) + getSocketErrorString());
		return false;
	}
	return true;	
}

bool UDPv4Socket::setMulticastTTL(uint8_t ttl)
{
	int ttl2,status;

	ttl2 = (int)ttl;
	status = setsockopt(m_sock, IPPROTO_IP, IP_MULTICAST_TTL, (const char *)&ttl2, sizeof(int));
	if (status != 0)
	{
		setErrorString(std::string(UDPV4SOCKET_ERRSTR_SETSOCKOPTERROR) + getSocketErrorString());
		return false;
	}
	return true;
}
	
bool UDPv4Socket::write(const void *data, size_t &length, const NetworkLayerAddress &destinationAddress, uint16_t destinationPort)
{
	if (m_sock == NUTSOCKERR)
	{
		setErrorString(UDPV4SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	if (destinationAddress.getProtocol() != IPv4)
	{
		setErrorString(UDPV4SOCKET_ERRSTR_BADPROTOCOL);
		return false;
	}
	
	const IPv4Address &ip = (const IPv4Address &)destinationAddress;
#if (defined(WIN32) || defined(_WIN32_WCE))
	int x = (int)length;
	int y;
	int flags = 0;
#else
	size_t x = length;
	ssize_t y;
	int flags = MSG_NOSIGNAL;
#endif // WIN32 || _WIN32_WCE
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(struct sockaddr_in));;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ip.getAddress());
	addr.sin_port = htons(destinationPort);
	
	y = sendto(m_sock, (const char *)data, x, flags, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if (y == NUTSOCKERR)
	{
		setErrorString(std::string(UDPV4SOCKET_ERRSTR_CANTWRITE) + getSocketErrorString());
		return false;
	}
	length = (size_t)y;
	
	return true;
}

bool UDPv4Socket::getAvailableDataLength(size_t &length, bool &available)
{
	if (m_sock == NUTSOCKERR)
	{
		setErrorString(UDPV4SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

#if (defined(WIN32) || defined(_WIN32_WCE))
	unsigned long num = 0;
#else
	size_t num = 0;
#endif // WIN32 || _WIN32_WCE

	if (NUTIOCTL(m_sock, FIONREAD, &num) != 0)
	{
		setErrorString(std::string(UDPV4SOCKET_ERRSTR_IOCTLERROR) + getSocketErrorString());
		return false;
	}

	length = (size_t)num;

	if (length > 0)
		available = true;
	else
	{
		struct sockaddr_in addr;
		uint8_t buf[1];
		socklen_t len = sizeof(struct sockaddr_in);

		memset(&addr,0,sizeof(struct sockaddr_in));

#if (defined(WIN32) || defined(_WIN32_WCE))
		bool b = m_isBlocking;
		
		if (b)
			setNonBlocking(true);
		recvfrom(m_sock, (char *)buf, 1, MSG_PEEK, (struct sockaddr *)&addr, &len);
		if (b)
			setNonBlocking(false);
#else
		recvfrom(m_sock, buf, 1, MSG_PEEK|MSG_DONTWAIT, (struct sockaddr *)&addr, &len);
#endif // WIN32 || _WIN32_WCE
		if (addr.sin_family == AF_INET)
			available = true;
		else
			available = false;
	}
	return true;
}

bool UDPv4Socket::read(void *buffer, size_t &bufferSize)
{
	if (m_sock == NUTSOCKERR)
	{
		setErrorString(UDPV4SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	if (!m_obtainDest)
	{
#if (defined(WIN32) || defined(_WIN32_WCE))
		int x = (int)bufferSize;
		int y;
		int addrLen = sizeof(struct sockaddr_in);
#else
		size_t x = bufferSize;
		ssize_t y;
		size_t addrLen = sizeof(struct sockaddr_in);
#endif // WIN32 || _WIN32_WCE
		struct sockaddr_in addr;

		if ((y = recvfrom(m_sock, (char *)buffer, x, 0, (struct sockaddr *)&addr, &addrLen)) == NUTSOCKERR)
		{
			setErrorString(std::string(UDPV4SOCKET_ERRSTR_CANTREAD) + getSocketErrorString());
			return false;
		}

		m_srcIP = IPv4Address(ntohl(addr.sin_addr.s_addr));
		m_srcPort = ntohs(addr.sin_port);
		
		bufferSize = (size_t)y;
	}
	else
	{
#define UDPV4SOCK_CONTROLLEN 2048	

#if (defined(WIN32) || defined(_WIN32_WCE))
#ifndef _WIN32_WCE
		WSAMSG mhdr;
		WSABUF buf;
		struct sockaddr_in addr;
		uint8_t control[UDPV4SOCK_CONTROLLEN];	

		buf.buf = (char *)buffer;
		buf.len = (u_long)bufferSize;

		mhdr.name = (SOCKADDR *)&addr;
		mhdr.namelen = sizeof(struct sockaddr_in);
		mhdr.lpBuffers = &buf;
		mhdr.dwBufferCount = 1;
		mhdr.Control.buf = (char *)control;
		mhdr.Control.len = UDPV4SOCK_CONTROLLEN;
		mhdr.dwFlags = 0;

		DWORD num = 0;

		if (WSARecvMsg(m_sock, &mhdr, &num, 0, 0) == NUTSOCKERR)
		{
			setErrorString(std::string(UDPV4SOCKET_ERRSTR_CANTREAD) + getSocketErrorString());
			return false;
		}

		bufferSize = (size_t)num;

		WSACMSGHDR *chdr = WSA_CMSG_FIRSTHDR(&mhdr);
		bool done = false;
		while (!done && chdr != 0)
		{
			if (chdr->cmsg_level == IPPROTO_IP && chdr->cmsg_type == IP_PKTINFO)
			{
				struct in_pktinfo *inf = (struct in_pktinfo *)WSA_CMSG_DATA(chdr);

				m_dstIP = IPv4Address(ntohl(inf->ipi_addr.s_addr));
				done = true;
			}
			else
				chdr = WSA_CMSG_NXTHDR(&mhdr,chdr);
		}
#else
		setErrorString(UDPV4SOCKET_ERRSTR_DESTADDRNOTSUPPORTED);
		return false;
#endif // _WIN32_WCE
#else
		struct msghdr mhdr;
		struct sockaddr_in addr;
		struct iovec vec;
		uint8_t control[UDPV4SOCK_CONTROLLEN];	

		vec.iov_base = buffer;
		vec.iov_len = bufferSize;
		
		memset(&addr,0,sizeof(struct sockaddr_in));
		
		mhdr.msg_name = &addr;
		mhdr.msg_namelen = sizeof(struct sockaddr_in);
		mhdr.msg_iov = &vec;
		mhdr.msg_iovlen = 1;
		mhdr.msg_control = &control;
		mhdr.msg_controllen = UDPV4SOCK_CONTROLLEN;
		mhdr.msg_flags = 0;
		
		ssize_t numReceived = recvmsg(m_sock, &mhdr, 0);
		if (numReceived == NUTSOCKERR)
		{
			setErrorString(std::string(UDPV4SOCKET_ERRSTR_CANTREAD) + getSocketErrorString());
			return false;
		}

		m_srcIP = IPv4Address(ntohl(addr.sin_addr.s_addr));
		m_srcPort = ntohs(addr.sin_port);
		bufferSize = (size_t)numReceived;

		struct cmsghdr *chdr = CMSG_FIRSTHDR(&mhdr);
		bool done = false;

		while (!done && chdr != 0)
		{
			if (chdr->cmsg_level == IPPROTO_IP && chdr->cmsg_type == IP_PKTINFO)
			{
				struct in_pktinfo *inf = (struct in_pktinfo *)CMSG_DATA(chdr);

				m_dstIP = IPv4Address(ntohl(inf->ipi_addr.s_addr));
				done = true;
			}
			else
				chdr = CMSG_NXTHDR(&mhdr,chdr);
		}
#endif // WIN32 || _WIN32_WCE
	}
	return true;
}

void UDPv4Socket::zeroAll()
{
	m_sock = NUTSOCKERR;
	m_localPort = 0;
	m_bindIP = 0;
	m_srcIP = IPv4Address((uint32_t)0);
	m_srcPort = 0;
}

bool UDPv4Socket::internalCreate(uint32_t ip, uint16_t port, bool obtainDestination)
{
	if (m_sock != NUTSOCKERR)
	{
		setErrorString(UDPV4SOCKET_ERRSTR_ALREADYCREATED);
		return false;
	}
	
	zeroAll();

#if (defined(WIN32) || defined(_WIN32_WCE))
	SOCKET s;
#else
	int s;
#endif // WIN32 || _WIN32_WCE
	
	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
	{
		setErrorString(std::string(UDPV4SOCKET_ERRSTR_CANTCREATESOCKET) + getSocketErrorString());
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
		setErrorString(std::string(UDPV4SOCKET_ERRSTR_CANTBIND) + getSocketErrorString());
		return false;
	}

	socklen_t addrLen = sizeof(struct sockaddr_in);
	memset(&addr, 0, addrLen);
	
	if (getsockname(s, (struct sockaddr *)&addr, &addrLen) != 0)
	{
		NUTCLOSE(s);
		setErrorString(std::string(UDPV4SOCKET_ERRSTR_CANTGETLOCALPORT) + getSocketErrorString());
		return false;
	}

	if (obtainDestination)
	{
#if (defined(WIN32) || defined(_WIN32_WCE))
#ifndef _WIN32_WCE
		DWORD NumberOfBytes;
		GUID WSARecvMsg_GUID = WSAID_WSARECVMSG;
		
		if (WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
			 &WSARecvMsg_GUID, sizeof WSARecvMsg_GUID,
			 &WSARecvMsg, sizeof WSARecvMsg,
			 &NumberOfBytes, NULL, NULL) == NUTSOCKERR)
		{
			NUTCLOSE(s);
			setErrorString(std::string(UDPV4SOCKET_ERRSTR_CANTGETRECVMSG) + getSocketErrorString());
			return false;
		}
		DWORD val = 1;
		if (setsockopt(s, IPPROTO_IP, IP_PKTINFO, (const char *)&val, sizeof(DWORD)) == NUTSOCKERR)
		{
			NUTCLOSE(s);
			setErrorString(std::string(UDPV4SOCKET_ERRSTR_CANTENABLEPKTINFO) + getSocketErrorString());
			return false;
		}
#else
		NUTCLOSE(s);
		setErrorString(UDPV4SOCKET_ERRSTR_DESTADDRNOTSUPPORTED);
		return false;
#endif // _WIN32_WCE
#else
		int val = 1;
		if (setsockopt(s, IPPROTO_IP, IP_PKTINFO, &val, sizeof(int)) == NUTSOCKERR)
		{
			NUTCLOSE(s);
			setErrorString(std::string(UDPV4SOCKET_ERRSTR_CANTENABLEPKTINFO) + getSocketErrorString());
			return false;
		}
#endif // WIN32 || _WIN32_WCE
		m_obtainDest = true;
	}
	else
		m_obtainDest = false;

	m_sock = s;
	m_localPort = ntohs(addr.sin_port);
	m_bindIP = 0;
	m_isBlocking = true;
	
	return true;
	
}

bool UDPv4Socket::waitForData(int seconds, int microSeconds)
{
	if (m_sock == NUTSOCKERR)
	{
		setErrorString(UDPV4SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

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
		setErrorString(std::string(UDPV4SOCKET_ERRSTR_CANTSELECT) + getSocketErrorString());
		return false;
	}
	
	return true;
}

std::string UDPv4Socket::getSocketErrorString()
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

