#ifndef NUT_UDPV4SOCKET_H

#define NUT_UDPV4SOCKET_H

#include "nutconfig.h"
#include "udpsocket.h"
#include "ipv4address.h"

#if defined(WIN32) && !defined(_WIN32_WCE)
	#include <mswsock.h>
#endif // WIN32

namespace nut
{

class UDPv4Socket : public UDPSocket
{
public:
	UDPv4Socket();
	UDPv4Socket(const std::string &objName);
	~UDPv4Socket();

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
#if (defined(WIN32) || defined(_WIN32_WCE))
	SOCKET getSocketDescriptor()									{ return m_sock; }
#else	
	int getSocketDescriptor()									{ return m_sock; }
#endif // WIN32 || _WIN32_WCE
private:
	void zeroAll();
	bool internalCreate(uint32_t ip, uint16_t port, bool obtainDestination);
	std::string getSocketErrorString();

#if (defined(WIN32) || defined(_WIN32_WCE))
	SOCKET m_sock;

#ifndef _WIN32_WCE
	LPFN_WSARECVMSG WSARecvMsg;
#endif // _WIN32_WCE
#else
	int m_sock;
#endif // WIN32 || _WIN32_WCE
	
	bool m_isBlocking;
	uint16_t m_localPort;
	uint32_t m_bindIP;
	IPv4Address m_srcIP;
	uint16_t m_srcPort;
	IPv4Address m_dstIP;
	bool m_obtainDest;
};

} // end namespace

#endif // NUT_UDPV4SOCKET_H

