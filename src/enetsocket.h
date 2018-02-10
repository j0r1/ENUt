#ifndef NUT_ENETSOCKET_H

#define NUT_ENETSOCKET_H

#include "nutconfig.h"
#include "socket.h"
#include "ipv4address.h"
#include <enet/enet.h>
#include <map>
#include <list>

namespace nut
{

class ENETSocket : public Socket
{
public:
	ENETSocket();
	ENETSocket(const std::string &objName);
	~ENETSocket();

	static bool ENETStartup();
	static void ENETCleanup();

	bool createServer(int maxConnections, uint16_t port = 0);
	bool createServer(const IPv4Address &bindAddr, int maxConnections, uint16_t port = 0);
	bool createClient(int maxConnections);

	bool isServer() const										{ return m_server; }
	bool close();
	
	uint16_t getLocalPort() const									{ return m_localPort; }

	bool waitForEvent(int milliseconds);
	bool poll();
		
	bool requestConnection(const IPv4Address &destAddress, uint16_t destPort, uint8_t channelCount);
	bool closeConnection(uint32_t connID);

	int getNumberOfConnections()									{ return (int)m_connectionMap.size(); }
	const uint32_t *getConnectionIDs()								{ return m_pConnectionIDs; }
	bool getConnectionInfo(uint32_t connID, IPv4Address &addr, uint16_t &port);
	
	bool write(const void *pData, size_t dataLength, bool reliable = false, uint8_t channel = 0, uint32_t connID = 0, bool allExceptConnID = false);
	bool getAvailableDataLength(size_t &length, bool &available);
	bool read(void *buffer, size_t &bufferSize, uint8_t *channel = 0, uint32_t *connID = 0);
protected:
	virtual void onNewConnection(uint32_t connID, const IPv4Address &sourceAddress, uint16_t sourcePort)	{ }
	virtual void onCloseConnection(uint32_t connID)								{ }
		
	ENetSocket getSocketDescriptor()								{ return m_pHost->socket; }
private:
	bool createServerInternal(const ENetAddress &bindAddress, int maxConnections);
	bool createClientInternal(uint32_t bindIP, uint16_t bindPort, int maxConnections);
	void buildNewConnectionArray();
			
	ENetHost *m_pHost;
	bool m_server;
	uint16_t m_localPort;

	class ENETPacket
	{
	public: 
		ENETPacket(ENetPacket *pPacket, uint8_t channel, uint32_t connID)
		{
			m_pPacket = pPacket;
			m_channel = channel;
			m_connID = connID;
		}

		ENetPacket *getPacket()									{ return m_pPacket; }
		uint8_t getChannel() const								{ return m_channel; }
		uint32_t getConnectionID() const							{ return m_connID; }
	private:
		ENetPacket *m_pPacket;
		uint8_t m_channel;
		uint32_t m_connID;
	};

	std::list<ENETPacket> m_packetQueue;

	uint32_t m_nextConnID;
	std::map<uint32_t, ENetPeer *> m_connectionMap;	
	uint32_t *m_pConnectionIDs;
	uint32_t m_maxConnections;

	friend class MulticastTunnelSocket;
};

};

#endif // NUT_ENETSOCKET_H

