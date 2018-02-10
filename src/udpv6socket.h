#ifndef NUT_UDPV6SOCKET_H

#define NUT_UDPV6SOCKET_H

#include "nutconfig.h"

#ifdef NUTCONFIG_SUPPORT_IPV6

#include "udpsocket.h"
#include "ipv6address.h"
#include <netinet/in.h>

namespace nut
{

class UDPv6Socket : public UDPSocket
{
public:
	UDPv6Socket();
	UDPv6Socket(const std::string &objName);
	~UDPv6Socket();

	bool create(uint16_t portNumber = 0, bool obtainDestination = false);
	bool create(NetworkLayerAddress &bindAddress, uint16_t portNumber = 0, bool obtainDestination = false);
	bool destroy();
	
	bool setNonBlocking(bool f = true);
	
	uint16_t getLocalPortNumber()									{ return m_localPort; }
	
	bool joinMulticastGroup(const NetworkLayerAddress &groupAddress);
	bool leaveMulticastGroup(const NetworkLayerAddress &groupAddress);
	bool setMulticastTTL(uint8_t ttl);
	
	bool waitForData(int seconds = -1, int microSeconds = -1); 	
	bool write(const void *data, size_t &length, const NetworkLayerAddress &destinationAddress, 
	           uint16_t destinationPort);
	bool getAvailableDataLength(size_t &length, bool &available);
	bool read(void *buffer, size_t &bufferSize);
	const NetworkLayerAddress *getLastSourceAddress()						{ return &m_srcIP; }
	uint16_t getLastSourcePort()									{ return m_srcPort; }
	const NetworkLayerAddress *getLastDestinationAddress()						{ return &m_dstIP; }
protected:
	int getSocketDescriptor()									{ return m_sock; }
private:
	void zeroAll();
	bool internalCreate(in6_addr ip, uint16_t port, bool obtainDestination);

	int m_sock;
	uint16_t m_localPort;
	IPv6Address m_srcIP;
	uint16_t m_srcPort;
	IPv6Address m_dstIP;
	bool m_obtainDest;
};

} // end namespace

#endif // NUTCONFIG_SUPPORT_IPV6

#endif // NUT_UDPV6SOCKET_H

