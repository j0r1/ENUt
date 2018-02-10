#include "nutconfig.h"

#ifdef NUTCONFIG_SUPPORT_IPV6

#include "udpv6socket.h"
#include "ipv6address.h"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

#define UDPV6SOCKET_ERRSTR_ALREADYCREATED					"Socket is already created"
#define UDPV6SOCKET_ERRSTR_NOTCREATED						"Socket is not yet created"
#define UDPV6SOCKET_ERRSTR_BADPROTOCOL						"Specified address is not an IPv6 address"
#define UDPV6SOCKET_ERRSTR_IOCTLERROR						"Error in 'ioctl' call: "
#define UDPV6SOCKET_ERRSTR_CANTWRITE						"Error in 'sendto' call: "
#define UDPV6SOCKET_ERRSTR_CANTREAD						"Error in 'recvfrom' call: "
#define UDPV6SOCKET_ERRSTR_CANTCREATESOCKET					"Error in 'socket' call: "
#define UDPV6SOCKET_ERRSTR_CANTBIND						"Error in 'bind' call: "
#define UDPV6SOCKET_ERRSTR_CANTGETLOCALPORT					"Error in 'getsockname' call: "
#define UDPV6SOCKET_ERRSTR_NOTMCASTADDR						"Specified address is not a multicast address"
#define UDPV6SOCKET_ERRSTR_SETSOCKOPTERROR					"Error in 'setsockopt' call: "
#define UDPV6SOCKET_ERRSTR_CANTSELECT						"Error in 'select' call: "
#define UDPV6SOCKET_ERRSTR_CANTSETPKTINFO					"Can't set IPV6_PKTINFO: "

namespace nut
{

UDPv6Socket::UDPv6Socket() : UDPSocket(IPv6)
{
	zeroAll();
}

UDPv6Socket::UDPv6Socket(const std::string &objName) : UDPSocket(objName, IPv6)
{
	zeroAll();
}

UDPv6Socket::~UDPv6Socket()
{
	destroy();
}

bool UDPv6Socket::create(uint16_t portNumber, bool obtainDestination)
{
	return internalCreate(in6addr_any, portNumber, obtainDestination);
}

bool UDPv6Socket::create(NetworkLayerAddress &bindAddress, uint16_t portNumber, bool obtainDestination)
{
	if (bindAddress.getProtocol() != IPv6)
	{
		setErrorString(UDPV6SOCKET_ERRSTR_BADPROTOCOL);
		return false;
	}
	
	const IPv6Address &ip = (const IPv6Address &)bindAddress;

	return internalCreate(ip.getAddress(), portNumber, obtainDestination);
}

bool UDPv6Socket::destroy()
{
	if (m_sock == -1)
	{
		setErrorString(UDPV6SOCKET_ERRSTR_NOTCREATED);
		return false;
	}

	close(m_sock);
	zeroAll();
	
	return true;
}
	
bool UDPv6Socket::setNonBlocking(bool f)
{
	if (m_sock == -1)
	{
		setErrorString(UDPV6SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	
	
	int flag = 0;

	if (f)
		flag = 1;
	
	if (ioctl(m_sock, FIONBIO, &flag) == -1)
	{
		setErrorString(std::string(UDPV6SOCKET_ERRSTR_IOCTLERROR) + std::string(strerror(errno)));
		return false;
	}
	
 	return true;
}

#define UDPV6SOCKET_IS_MCASTADDR(x)					(x.s6_addr[0] == 0xFF)

// TODO: use ipv6mr_interface properly
#define UDPV6SOCKET_MCASTMEMBERSHIP(socket,type,mcastip,status)		{\
										struct ipv6_mreq mreq;\
										\
										mreq.ipv6mr_multiaddr = mcastip;\
										mreq.ipv6mr_interface = 0;\
										status = setsockopt(socket,IPPROTO_IPV6,type,(const char *)&mreq,sizeof(struct ipv6_mreq));\
									}
	
bool UDPv6Socket::joinMulticastGroup(const NetworkLayerAddress &groupAddress)
{
	if (m_sock == -1)
	{
		setErrorString(UDPV6SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	if (groupAddress.getProtocol() != IPv6)
	{
		setErrorString(UDPV6SOCKET_ERRSTR_BADPROTOCOL);
		return false;
	}

	const IPv6Address &ip = (const IPv6Address &)groupAddress;
	int status;

	if (!UDPV6SOCKET_IS_MCASTADDR(ip.getAddress()))
	{
		setErrorString(UDPV6SOCKET_ERRSTR_NOTMCASTADDR);
		return false;
	}

	UDPV6SOCKET_MCASTMEMBERSHIP(m_sock, IP_ADD_MEMBERSHIP, ip.getAddress(), status);
	if (status != 0)
	{
		setErrorString(std::string(UDPV6SOCKET_ERRSTR_SETSOCKOPTERROR) + std::string(strerror(errno)));
		return false;
	}
	
	return true;
}

bool UDPv6Socket::leaveMulticastGroup(const NetworkLayerAddress &groupAddress)
{
	if (m_sock == -1)
	{
		setErrorString(UDPV6SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	if (groupAddress.getProtocol() != IPv6)
	{
		setErrorString(UDPV6SOCKET_ERRSTR_BADPROTOCOL);
		return false;
	}

	const IPv6Address &ip = (const IPv6Address &)groupAddress;
	int status;

	if (!UDPV6SOCKET_IS_MCASTADDR(ip.getAddress()))
	{
		setErrorString(UDPV6SOCKET_ERRSTR_NOTMCASTADDR);
		return false;
	}

	UDPV6SOCKET_MCASTMEMBERSHIP(m_sock, IP_DROP_MEMBERSHIP, ip.getAddress(), status);
	if (status != 0)
	{
		setErrorString(std::string(UDPV6SOCKET_ERRSTR_SETSOCKOPTERROR) + std::string(strerror(errno)));
		return false;
	}
	return true;	
}

bool UDPv6Socket::setMulticastTTL(uint8_t ttl)
{
	int ttl2, status;

	ttl2 = (int)ttl;
	status = setsockopt(m_sock, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (const char *)&ttl2, sizeof(int));
	if (status != 0)
	{
		setErrorString(std::string(UDPV6SOCKET_ERRSTR_SETSOCKOPTERROR) + std::string(strerror(errno)));
		return false;
	}
	return true;
}
	
bool UDPv6Socket::write(const void *data, size_t &length, const NetworkLayerAddress &destinationAddress, uint16_t destinationPort)
{
	if (m_sock == -1)
	{
		setErrorString(UDPV6SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	if (destinationAddress.getProtocol() != IPv6)
	{
		setErrorString(UDPV6SOCKET_ERRSTR_BADPROTOCOL);
		return false;
	}
	
	const IPv6Address &ip = (const IPv6Address &)destinationAddress;
	size_t x;
	ssize_t y;
	struct sockaddr_in6 addr;

	memset(&addr, 0, sizeof(struct sockaddr_in6));;
	addr.sin6_family = AF_INET6;
	addr.sin6_addr = ip.getAddress();
	addr.sin6_port = htons(destinationPort);
	
	x = length;
	y = sendto(m_sock, data, x, MSG_NOSIGNAL, (struct sockaddr *)&addr, sizeof(struct sockaddr_in6));
	if (y == -1)
	{
		setErrorString(std::string(UDPV6SOCKET_ERRSTR_CANTWRITE) + std::string(strerror(errno)));
		return false;
	}
	length = (size_t)y;
	
	return true;
}

bool UDPv6Socket::getAvailableDataLength(size_t &length, bool &available)
{
	if (m_sock == -1)
	{
		setErrorString(UDPV6SOCKET_ERRSTR_NOTCREATED);
		return false;
	}	

	size_t num;

	if (ioctl(m_sock, FIONREAD, &num) != 0)
	{
		setErrorString(std::string(UDPV6SOCKET_ERRSTR_IOCTLERROR) + std::string(strerror(errno)));
		return false;
	}

	length = num;
	if (length > 0)
		available = true;
	else
	{
		struct sockaddr_in6 addr;
		uint8_t buf[1];
		socklen_t len = sizeof(struct sockaddr_in6);

		memset(&addr,0,sizeof(struct sockaddr_in6));
		recvfrom(m_sock, buf, 1, MSG_PEEK|MSG_DONTWAIT, (struct sockaddr *)&addr, &len);

		if (addr.sin6_family == AF_INET6)
			available = true;
		else
			available = false;
	}

	return true;
}

bool UDPv6Socket::read(void *buffer, size_t &bufferSize)
{
	if (m_sock == -1)
	{
		setErrorString(UDPV6SOCKET_ERRSTR_NOTCREATED);
		return false;
	}		

	if (!m_obtainDest)
	{
		struct sockaddr_in6 addr;
		size_t addrLen = sizeof(struct sockaddr_in6);
		size_t x = bufferSize;
		ssize_t y;

		if ((y = recvfrom(m_sock, buffer, x, 0, (struct sockaddr *)&addr, &addrLen)) == -1)
		{
			setErrorString(std::string(UDPV6SOCKET_ERRSTR_CANTREAD) + std::string(strerror(errno)));
			return false;
		}

		m_srcIP = IPv6Address(addr.sin6_addr);
		m_srcPort = ntohs(addr.sin6_port);
		
		bufferSize = (size_t)y;
	}
	else
	{
		struct msghdr mhdr;
		struct sockaddr_in6 addr;
		struct iovec vec;

#define UDPV6SOCK_CONTROLLEN 2048
	
		uint8_t control[UDPV6SOCK_CONTROLLEN];
		
		vec.iov_base = buffer;
		vec.iov_len = bufferSize;
		
		memset(&addr,0,sizeof(struct sockaddr_in6));
		
		mhdr.msg_name = &addr;
		mhdr.msg_namelen = sizeof(struct sockaddr_in6);
		mhdr.msg_iov = &vec;
		mhdr.msg_iovlen = 1;
		mhdr.msg_control = &control;
		mhdr.msg_controllen = UDPV6SOCK_CONTROLLEN;
		mhdr.msg_flags = 0;
		
		ssize_t numReceived = recvmsg(m_sock, &mhdr, 0);
		if (numReceived == -1)
		{
			setErrorString(std::string(UDPV6SOCKET_ERRSTR_CANTREAD) + getSocketErrorString());
			return false;
		}

		m_srcIP = IPv6Address(addr.sin6_addr);
		m_srcPort = ntohs(addr.sin6_port);
		bufferSize = (size_t)numReceived;

		struct cmsghdr *chdr = CMSG_FIRSTHDR(&mhdr);
		bool done = false;

		while (!done && chdr != 0)
		{
			if (chdr->cmsg_level == IPPROTO_IPV6 && chdr->cmsg_type == IPV6_PKTINFO)
			{
				struct in_pktinfo *inf = (struct in_pktinfo *)CMSG_DATA(chdr);

				m_dstIP = IPv6Address(inf->ipi_addr);
				done = true;
			}
			else
				chdr = CMSG_NXTHDR(&mhdr,chdr);
		}
	}
	
	return true;
}

void UDPv6Socket::zeroAll()
{
	m_sock = -1;
	m_localPort = 0;
	m_srcPort = 0;
	m_srcIP = IPv6Address();
}

bool UDPv6Socket::internalCreate(in6_addr ip, uint16_t port, bool obtainDestination)
{
	if (m_sock != -1)
	{
		setErrorString(UDPV6SOCKET_ERRSTR_ALREADYCREATED);
		return false;
	}
	
	zeroAll();

	int s;
	
	if ((s = socket(PF_INET6, SOCK_DGRAM, 0)) == -1)
	{
		setErrorString(std::string(UDPV6SOCKET_ERRSTR_CANTCREATESOCKET) + std::string(strerror(errno)));
		return false;
	}
	
	struct sockaddr_in6 addr;
	memset(&addr, 0, sizeof(struct sockaddr_in6));
	
	addr.sin6_family = AF_INET6;
	addr.sin6_addr = ip;
	addr.sin6_port = htons(port);

	if (bind(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_in6)) != 0)
	{
		close(s);
		setErrorString(std::string(UDPV6SOCKET_ERRSTR_CANTBIND) + std::string(strerror(errno)));
		return false;
	}

	socklen_t addrLen = sizeof(struct sockaddr_in6);
	memset(&addr, 0, addrLen);
	
	if (getsockname(s, (struct sockaddr *)&addr, &addrLen) != 0)
	{
		close(s);
		setErrorString(std::string(UDPV6SOCKET_ERRSTR_CANTGETLOCALPORT) + std::string(strerror(errno)));
		return false;
	}

	if (obtainDestination)
	{
		int val = 1;
		
		if (setsockopt(s, IPPROTO_IPV6, IPV6_PKTINFO, &val, sizeof(int)) == -1)
		{
			close(s);
			setErrorString(std::string(UDPV6SOCKET_ERRSTR_CANTSETPKTINFO) + std::string(strerror(errno)));
			return false;
		}
		m_obtainDest = true;
	}
	else 
		m_obtainDest = false;

	m_sock = s;
	m_localPort = ntohs(addr.sin6_port);
	
	return true;
	
}

bool UDPv6Socket::waitForData(int seconds, int microSeconds)
{
	if (m_sock == -1)
	{
		setErrorString(UDPV6SOCKET_ERRSTR_NOTCREATED);
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

	if (status == -1)
	{
		setErrorString(std::string(UDPV6SOCKET_ERRSTR_CANTSELECT) + std::string(strerror(errno)));
		return false;
	}
	
	return true;
}

} // end namespace

#endif // NUTCONFIG_SUPPORT_IPV6

