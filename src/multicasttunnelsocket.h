#ifndef NUT_MULTICASTTUNNELSOCKET_H

#define NUT_MULTICASTTUNNELSOCKET_H

#include "nutconfig.h"
#include "datagramsocket.h"
#include "enetsocket.h"
#include <list>

namespace nut
{

class IPv4Address;
class UDPSocket;
	
class MulticastTunnelSocket : public DatagramSocket
{
public:
	MulticastTunnelSocket();
	MulticastTunnelSocket(const std::string &objName);
	~MulticastTunnelSocket();
	
	bool create(const IPv4Address &serverAddr, uint16_t serverPort, uint16_t mcastRemotePortNumber);
	bool destroy();

	uint16_t getLocalPortNumber();

	bool joinMulticastGroup(const NetworkLayerAddress &groupAddress);
	bool leaveMulticastGroup(const NetworkLayerAddress &groupAddress);
	
	bool waitForData(int seconds = -1, int microSeconds = -1);
	bool write(const void *data, size_t &length, const NetworkLayerAddress &destinationAddress, uint16_t destinationPort);
	
	bool getAvailableDataLength(size_t &length, bool &available);
	bool read(void *buffer, size_t &bufferSize);
	const NetworkLayerAddress *getLastSourceAddress();
	uint16_t getLastSourcePort();
	const NetworkLayerAddress *getLastDestinationAddress();
protected:
	ENetSocket getSocketDescriptor()								{ return m_pEnetSocket->getSocketDescriptor(); }
private:
	class Packet
	{
	public:
		Packet(uint8_t *pBuf, size_t length, NetworkLayerAddress *pSrcAddr, uint16_t srcPort,
		       NetworkLayerAddress *pDstAddr)
		{
			m_pBuf = pBuf;
			m_length = length;
			m_pSrcAddr = pSrcAddr;
			m_srcPort = srcPort;
			m_pDstAddr = pDstAddr;
		}

		~Packet()
		{
			delete [] m_pBuf;
			delete m_pSrcAddr;
			delete m_pDstAddr;
		}

		uint8_t *getBuffer() const								{ return m_pBuf + sizeof(uint16_t)+sizeof(uint32_t)*2; }
		size_t getLength() const								{ return m_length - sizeof(uint16_t)-sizeof(uint32_t)*2; }
		const NetworkLayerAddress *getSourceAddress() const					{ return m_pSrcAddr; }
		uint16_t getSourcePort() const								{ return m_srcPort; }
		const NetworkLayerAddress *getDestinationAddress() const				{ return m_pDstAddr; }
	private:
		uint8_t *m_pBuf;
		size_t m_length;
		NetworkLayerAddress *m_pSrcAddr, *m_pDstAddr;
		uint16_t m_srcPort;
	};
	
	bool poll();
	bool writeCommand(uint8_t cmdNum, uint8_t *pData = 0, size_t length = 0);
	
	ENETSocket *m_pEnetSocket;
	std::list<Packet *> m_packets;
	Packet *m_pLastReadPacket;
	bool m_init;
};
	
}

#endif // NUT_MULTICASTTUNNELSOCKET_H
