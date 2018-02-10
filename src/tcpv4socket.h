#ifndef NUT_TCPV4SOCKET_H

#define NUT_TCPV4SOCKET_H

#include "nutconfig.h"
#include "tcpsocket.h"
#include "ipv4address.h"

namespace nut
{

class TCPv4Socket : public TCPSocket
{
public:
	TCPv4Socket();
	TCPv4Socket(const std::string &objName);
	~TCPv4Socket();

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
#if (defined(WIN32) || defined(_WIN32_WCE))
	SOCKET getSocketDescriptor()								{ return m_sock; }
#else
	int getSocketDescriptor()								{ return m_sock; }
#endif // WIN32 || _WIN32_WCE
private:
	bool internalCreate(uint32_t ip, uint16_t port);
	void zeroAll();
	std::string getSocketErrorString();

#if (defined(WIN32) || defined(_WIN32_WCE))
	TCPv4Socket(SOCKET sock, uint16_t localPort, uint32_t destIP, uint16_t destPort);

	SOCKET m_sock;
#else
	TCPv4Socket(int sock, uint16_t localPort, uint32_t destIP, uint16_t destPort);

	int m_sock;
#endif // WIN32 || _WIN32_WCE

	bool m_connected, m_listening;
	uint16_t m_localPort;
	uint16_t m_destPort;
	IPv4Address *m_destIP;
};

}

#endif // NUT_TCPV4SOCKET_H
