#include "multicasttunnelsocket.h"
#include "enetsocket.h"
#include "ipv4address.h"
#include "multicasttunnelserver.h"
#include <time.h>
//#include <iostream>

namespace nut
{

inline void writeUint16(uint8_t *pBuf, uint16_t val)
{
	pBuf[0] = (uint8_t)(val&0xff);
	pBuf[1] = (uint8_t)((val>>8)&0xff);
}
	
inline void writeUint32(uint8_t *pBuf, uint32_t val)
{
	pBuf[0] = (uint8_t)(val&0xff);
	pBuf[1] = (uint8_t)((val>>8)&0xff);
	pBuf[2] = (uint8_t)((val>>16)&0xff);
	pBuf[3] = (uint8_t)((val>>24)&0xff);
}

uint16_t readUint16(uint8_t *pBuf)
{
	return (uint16_t)pBuf[0]|(((uint16_t)pBuf[1])<<8);
}

uint32_t readUint32(uint8_t *pBuf)
{
	return (uint32_t)pBuf[0]|(((uint32_t)pBuf[1])<<8)|(((uint32_t)pBuf[2])<<16)|(((uint32_t)pBuf[3])<<24);
}

MulticastTunnelSocket::MulticastTunnelSocket()
{
	m_init = false;
}

MulticastTunnelSocket::MulticastTunnelSocket(const std::string &objName) : DatagramSocket(objName)
{
	m_init = false;
}	

MulticastTunnelSocket::~MulticastTunnelSocket()
{
	destroy();
}
	
bool MulticastTunnelSocket::create(const IPv4Address &serverAddr, uint16_t serverPort, uint16_t mcastRemotePortNumber)
{
	if (m_init)
	{
		setErrorString("Already initialized");
		return false;
	}

	m_pLastReadPacket = 0;
	m_pEnetSocket = new ENETSocket();

	if (!m_pEnetSocket->createClient(1))
	{
		setErrorString(m_pEnetSocket->getErrorString());
		return false;
	}

	// try to connect to multicast tunnel server
	
	if (!m_pEnetSocket->requestConnection(serverAddr, serverPort, 2))
	{
		setErrorString(m_pEnetSocket->getErrorString());
		delete m_pEnetSocket;
		return false;
	}
	
	time_t startTime = time(0);
	
	while (m_pEnetSocket->getNumberOfConnections() != 1 && (time(0) - startTime) < 10)
	{
		m_pEnetSocket->waitForEvent(100);
		m_pEnetSocket->poll();
	}

	if (m_pEnetSocket->getNumberOfConnections() != 1)
	{
		setErrorString("Timeout while trying to connect to multicast tunnel server");
		return false;
	}

	// ok, we're connected; write remote multicast port and wait for ack
	
	uint8_t buf[sizeof(uint16_t)];
	
	writeUint16(buf,mcastRemotePortNumber);
	
	if (!writeCommand(MCASTTUNNEL_COMMAND_PORT, buf, sizeof(uint16_t)))
	{
		delete m_pEnetSocket;
		return false;
	}

	startTime = time(0);
	size_t length = 0;
	bool avail = false;
	
	while (!avail && (time(0) - startTime) < 10)
	{
		m_pEnetSocket->waitForEvent(100);
		m_pEnetSocket->poll();
		m_pEnetSocket->getAvailableDataLength(length,avail);
	}

	if (length <= 0)
	{
		setErrorString("Timeout while waiting for port acknowledgment");
		delete m_pEnetSocket;
		return false;
	}

	if (length != sizeof(uint8_t))
	{
		setErrorString("Invalid length while waiting for ack command");
		delete m_pEnetSocket;
		return false;
	}

	uint8_t channel;
	
	m_pEnetSocket->read(buf, length, &channel);
	if (channel != MCASTTUNNEL_CHANNEL_CONTROL)
	{
		setErrorString("Invalid channel while waiting for ack command");
		delete m_pEnetSocket;
		return false;
	}
			
	if (buf[0] == MCASTTUNNEL_COMMAND_ACK)
	{
		// ok, just continue
	}
	else if (buf[0] == MCASTTUNNEL_COMMAND_ACKERR)
	{
		setErrorString("Multicast tunnel server couldn't open requested port");
		delete m_pEnetSocket;
		return false;
	}
	else
	{
		setErrorString("Got invalid command while waiting for port ack");
		delete m_pEnetSocket;
		return false;
	}
	
	// ok, ack received, send start command
	
	writeCommand(MCASTTUNNEL_COMMAND_START);
	
	m_init = true;
	return true;
}

bool MulticastTunnelSocket::destroy()
{
	if (!m_init)
	{
		setErrorString("Not initialized");
		return false;
	}

	if (m_pEnetSocket->getNumberOfConnections() > 0)
	{
		time_t startTime = time(0);
		bool disconnected = false;
		
		m_pEnetSocket->closeConnection(m_pEnetSocket->getConnectionIDs()[0]);
	
		while (!disconnected && (time(0) - startTime) < 3)
		{
			m_pEnetSocket->poll();
		
			if (m_pEnetSocket->getNumberOfConnections() == 0)
				disconnected = true;
			else
#if defined(WIN32) || defined(_WIN32_WCE)
				Sleep(1000);
#else
				sleep(1);
#endif // WIN32 || _WIN32_WCE
		}
	}
	
	delete m_pEnetSocket;
	std::list<Packet *>::iterator it;

	for (it = m_packets.begin() ; it != m_packets.end() ; it++)
		delete (*it);

	m_packets.clear();
	if (m_pLastReadPacket)
		delete m_pLastReadPacket;
	m_init = false;

	return true;
}

uint16_t MulticastTunnelSocket::getLocalPortNumber()
{
	return m_pEnetSocket->getLocalPort();
}

bool MulticastTunnelSocket::joinMulticastGroup(const NetworkLayerAddress &groupAddress)
{
	if (groupAddress.getProtocol() != nut::IPv4)
	{
		setErrorString("Only IPv4 is supported");
		return false;
	}

	if (!m_init)
	{
		setErrorString("Not initialized");
		return false;
	}
	
	if (m_pEnetSocket->getNumberOfConnections() == 0)
	{
		setErrorString("Server connection lost");
		return false;
	}

	const IPv4Address &addr = (const IPv4Address &)groupAddress;
	uint8_t buf[sizeof(uint32_t)];

	writeUint32(buf, addr.getAddress());

	if (!writeCommand(MCASTTUNNEL_COMMAND_JOIN, buf, sizeof(uint32_t)))
		return false;

	return true;
}

bool MulticastTunnelSocket::leaveMulticastGroup(const NetworkLayerAddress &groupAddress)
{
	if (groupAddress.getProtocol() != nut::IPv4)
	{
		setErrorString("Only IPv4 is supported");
		return false;
	}

	if (!m_init)
	{
		setErrorString("Not initialized");
		return false;
	}
	
	if (m_pEnetSocket->getNumberOfConnections() == 0)
	{
		setErrorString("Server connection lost");
		return false;
	}
	
	const IPv4Address &addr = (const IPv4Address &)groupAddress;
	uint8_t buf[sizeof(uint32_t)];

	writeUint32(buf, addr.getAddress());

	if (!writeCommand(MCASTTUNNEL_COMMAND_LEAVE, buf, sizeof(uint32_t)))
		return false;

	return true;
}

bool MulticastTunnelSocket::waitForData(int seconds, int microSeconds)
{
	if (!m_init)
	{
		setErrorString("Not initialized");
		return false;
	}
	
	if (m_pEnetSocket->getNumberOfConnections() == 0)
	{
		setErrorString("Server connection lost");
		return false;
	}
	
	int millisec;
	
	if (seconds >= 0 && microSeconds >= 0)
		millisec = microSeconds/1000+seconds*1000;
	else
		millisec = 0;

	if (!m_pEnetSocket->waitForEvent(millisec))
	{
		setErrorString(m_pEnetSocket->getErrorString());
		return false;
	}
	return true;
}

#define IS_MCASTADDR(x)							(((x)&0xF0000000) == 0xE0000000)

bool MulticastTunnelSocket::write(const void *data, size_t &length, const NetworkLayerAddress &destinationAddress, uint16_t destinationPort)
{
	//std::cerr << getObjectName() << ": Entering write" << std::endl;
	if (destinationAddress.getProtocol() != nut::IPv4)
	{
		setErrorString("Only IPv4 is supported");
		//std::cerr << getObjectName() << ": Leaving write 1" << std::endl;
		return false;
	}

	if (!m_init)
	{
		setErrorString("Not initialized");
		//std::cerr << getObjectName() << ": Leaving write 2" << std::endl;
		return false;
	}
	
	if (m_pEnetSocket->getNumberOfConnections() == 0)
	{
		setErrorString("Server connection lost");
		//std::cerr << getObjectName() << ": Leaving write 3" << std::endl;
		return false;
	}

	const IPv4Address &addr = (const IPv4Address &)destinationAddress;

	uint32_t ip = addr.getAddress();
	if (!IS_MCASTADDR(ip))
	{
		setErrorString("Not a multicast address");
		//std::cerr << getObjectName() << ": Leaving write 4" << std::endl;
		return false;
	}

	uint8_t *pBuf = new uint8_t[length + sizeof(uint32_t) + sizeof(uint16_t)];

	memcpy(pBuf+sizeof(uint32_t)+sizeof(uint16_t),data,length);

	writeUint32(pBuf, ip);
	writeUint16(pBuf+sizeof(uint32_t), destinationPort);

	if (!m_pEnetSocket->write(pBuf, sizeof(uint32_t)+sizeof(uint16_t)+length, false, MCASTTUNNEL_CHANNEL_DATA))
	{
		delete [] pBuf;
		setErrorString(m_pEnetSocket->getErrorString());
		//std::cerr << getObjectName() << ": Leaving write 5" << std::endl;
		return false;
	}

	delete [] pBuf;
	//std::cerr << getObjectName() << ": Leaving write ok" << std::endl;
	
	return true;
}
	
bool MulticastTunnelSocket::getAvailableDataLength(size_t &length, bool &available)
{
	//std::cerr << getObjectName() << ": Entering getAvailableDataLength" << std::endl;
	if (!m_init)
	{
		setErrorString("Not initialized");
		//std::cerr << getObjectName() << ": Leaving getAvailableDataLength 1" << std::endl;
		return false;
	}
	
	if (m_pEnetSocket->getNumberOfConnections() == 0)
	{
		setErrorString("Server connection lost");
		//std::cerr << getObjectName() << ": Leaving getAvailableDataLength 2" << std::endl;
		return false;
	}

	if (!poll())
	{
		//std::cerr << getObjectName() << ": Leaving getAvailableDataLength 3" << std::endl;
		return false;
	}

	if (m_packets.empty())
	{
		length = 0;
		available = false;
		//std::cerr << getObjectName() << ": Leaving getAvailableDataLength ok 1" << std::endl;
	}
	else
	{
		length = (*(m_packets.begin()))->getLength();
		available = true;
		//std::cerr << getObjectName() << ": Leaving getAvailableDataLength ok 2" << std::endl;
	}
	return true;
}

bool MulticastTunnelSocket::read(void *buffer, size_t &bufferSize)
{
	//std::cerr << getObjectName() << ": Entering read" << std::endl;
	if (!m_init)
	{
		setErrorString("Not initialized");
		//std::cerr << getObjectName() << ": Leaving read 1" << std::endl;
		return false;
	}
	
	if (m_pEnetSocket->getNumberOfConnections() == 0)
	{
		setErrorString("Server connection lost");
		//std::cerr << getObjectName() << ": Leaving read 2" << std::endl;
		return false;
	}

	if (!poll())
	{
		//std::cerr << getObjectName() << ": Leaving read 3" << std::endl;
		return false;
	}

	if (m_packets.empty())
	{
		setErrorString("No data available");
		//std::cerr << getObjectName() << ": Leaving read 4" << std::endl;
		return false;
	}
	
	size_t length = (*(m_packets.begin()))->getLength();

	if (length > bufferSize)
	{
		bufferSize = length;
		setErrorString("Buffer size not large enough");
		//std::cerr << getObjectName() << ": Leaving read 5" << std::endl;
		return false;
	}
	
	if (m_pLastReadPacket)
		delete m_pLastReadPacket;
	m_pLastReadPacket = *(m_packets.begin());
	m_packets.pop_front();

	memcpy(buffer, m_pLastReadPacket->getBuffer(), length);
	bufferSize = length;
	
	//std::cerr << getObjectName() << ": Leaving read ok" << std::endl;
	
	return true;
}

const NetworkLayerAddress *MulticastTunnelSocket::getLastSourceAddress()
{
	if (!m_init || !m_pLastReadPacket)
		return 0;
	return m_pLastReadPacket->getSourceAddress();
}

uint16_t MulticastTunnelSocket::getLastSourcePort()
{
	if (!m_init || !m_pLastReadPacket)
		return 0;
	return m_pLastReadPacket->getSourcePort();
}

const NetworkLayerAddress *MulticastTunnelSocket::getLastDestinationAddress()
{
	if (!m_init || !m_pLastReadPacket)
		return 0;
	return m_pLastReadPacket->getDestinationAddress();
}

bool MulticastTunnelSocket::poll()
{
	//std::cerr << getObjectName() << ": Entering poll" << std::endl;
	if (!m_pEnetSocket->poll())
	{
		setErrorString(m_pEnetSocket->getErrorString());
		//std::cerr << getObjectName() << ": Leaving poll 1" << std::endl;
		return false;
	}

	size_t length = 0;
	bool avail = false;

	//std::cerr << getObjectName() << ": poll: getting first available data" << std::endl;
	m_pEnetSocket->getAvailableDataLength(length,avail);
	//std::cerr << getObjectName() << ": available data is " << length << ((avail)?"yes":"no") << std::endl;
	
	while (avail)
	{
		uint8_t *pBuf = new uint8_t [length + 1]; // avoid length = 0
		uint8_t channel;
		size_t l = length;
		
		if (!m_pEnetSocket->read(pBuf,l,&channel))
		{
			delete [] pBuf;
			setErrorString(m_pEnetSocket->getErrorString());
			//std::cerr << getObjectName() << ": Leaving poll 2" << std::endl;
			return false;
		}

		if (channel != MCASTTUNNEL_CHANNEL_DATA)
		{
			delete [] pBuf;
			setErrorString("Received data on invalid channel");
			//std::cerr << getObjectName() << ": Leaving poll 3" << std::endl;
			return false;
		}
		if (length < sizeof(uint16_t)+sizeof(uint32_t)*2)
		{
			delete [] pBuf;
			setErrorString("Received data with invalid size");
			//std::cerr << getObjectName() << ": Leaving poll 4" << std::endl;
			return false;
		}

		uint32_t srcIP = readUint32(pBuf);
		uint16_t srcPort = readUint16(pBuf+sizeof(uint32_t));
		uint32_t dstIP = readUint32(pBuf+sizeof(uint32_t)+sizeof(uint16_t));
		
		Packet *pPack = new Packet(pBuf,l,new IPv4Address(srcIP),srcPort,new IPv4Address(dstIP));
		m_packets.push_back(pPack);

		length = 0;
		avail = false;
		//std::cerr << getObjectName() << ": poll: getting loop available data" << std::endl;
		m_pEnetSocket->getAvailableDataLength(length,avail);
		//std::cerr << getObjectName() << ": available data is " << length << ((avail)?"yes":"no") << std::endl;
	}

	//std::cerr << getObjectName() << ": Leaving poll ok" << std::endl;
	
	return true;
}

bool MulticastTunnelSocket::writeCommand(uint8_t cmdNum, uint8_t *pData, size_t length)
{
	uint8_t *pBuf = new uint8_t[length+1];

	pBuf[0] = cmdNum;
	if (length > 0)
		memcpy(pBuf+1,pData,length);

	if (!m_pEnetSocket->write(pBuf,length+1,true,MCASTTUNNEL_CHANNEL_CONTROL))
	{
		delete [] pBuf;
		setErrorString(m_pEnetSocket->getErrorString());
		return false;
	}

	delete [] pBuf;
	
	return true;
}

}
