#ifndef NUT_TCPV6SOCKET_H

#define NUT_TCPV6SOCKET_H

#include "nutconfig.h"

#ifdef NUTCONFIG_SUPPORT_IPV6

#include "tcpsocket.h"
#include "ipv6address.h"

namespace nut
{

class TCPv6Socket : public TCPSocket
{
public:
	TCPv6Socket();
	TCPv6Socket(const std::string &objName);
	~TCPv6Socket();

	bool create(uint16_t portNumber = 0);
	bool create(const NetworkLayerAddress &bindAddress, uint16_t portNumber = 0);
	bool destroy();

	bool setNonBlocking(bool f = true);
	
	uint16_t getLocalPortNumber()								{ return m_localPort; }

	bool connect(const NetworkLayerAddress &hostAddress, uint16_t portNumber);
	bool listen(int backlog);
	bool accept(TCPSocket **newsock);
	bool isConnected()									{ return m_connected; }
	
	NetworkLayerAddress *getDestinationAddress()						{ return m_destIP->createCopy(); }
	uint16_t getDestinationPort()								{ return m_destPort; }
	
	bool waitForData(int seconds = -1, int microSeconds = -1);
	bool write(const void *data, size_t &length);
	bool getAvailableDataLength(size_t &length);
	bool read(void *buffer, size_t &bufferSize);
protected:
	int getSocketDescriptor()								{ return m_sock; }
private:
	TCPv6Socket(int sock, uint16_t localPort, in6_addr destIP, uint16_t destPort);
	bool internalCreate(in6_addr ip, uint16_t port);
	void zeroAll();

	int m_sock;
	bool m_connected, m_listening;
	uint16_t m_localPort;
	uint16_t m_destPort;
	IPv6Address *m_destIP;
};

}

#endif // NUTCONFIG_SUPPORT_IPV6

#endif // NUT_TCPV6SOCKET_H

