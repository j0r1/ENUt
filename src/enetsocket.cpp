#include "nutconfig.h"
#include "enetsocket.h"

#define ENETSOCKET_ERRSTR_ALREADYCREATED				"The ENet socket is already initialized"
#define ENETSOCKET_ERRSTR_NOTCREATED					"The ENet socket has not been initialized yet"
#define ENETSOCKET_ERRSTR_CANTCREATESERVERHOST				"Error creating ENet server host"
#define ENETSOCKET_ERRSTR_CANTCREATECLIENTHOST				"Error creating ENet client host"
#define ENETSOCKET_ERRSTR_CANTGETLOCALPORT				"Unable to detect the local port being used"
#define ENETSOCKET_ERRSTR_ERRORINSERVICE				"Error in host service call"
#define ENETSOCKET_ERRSTR_CANTCONNECT					"Can't create a connection"
#define ENETSOCKET_ERRSTR_INVALIDCONNECTIONID				"Invalid connection ID"
#define ENETSOCKET_ERRSTR_NOPACKETAVAILABLE				"There's currently no packet available"
#define ENETSOCKET_ERRSTR_BUFFERSIZETOOSMALL				"Buffer size too small"
#define ENETSOCKET_ERRSTR_CANTBINDSOCKET				"Unable to bind client socket"

namespace nut
{

ENETSocket::ENETSocket()
{
	m_pHost = 0;
	m_server = false;
	m_localPort = 0;
}

ENETSocket::ENETSocket(const std::string &objName) : Socket(objName)
{
	m_pHost = 0;
	m_server = false;
	m_localPort = 0;
}

ENETSocket::~ENETSocket()
{
	close();
}
	
bool ENETSocket::ENETStartup()
{
	if (enet_initialize() != 0)
		return false;
	return true;
}

void ENETSocket::ENETCleanup()
{
	enet_deinitialize();
}

bool ENETSocket::createServer(int maxConnections, uint16_t port)
{
	ENetAddress addr;

	addr.host = ENET_HOST_ANY;
	addr.port = port;

	return createServerInternal(addr, maxConnections);
}

bool ENETSocket::createServer(const IPv4Address &bindAddr, int maxConnections, uint16_t port)
{
	ENetAddress addr;

	addr.host = ENET_HOST_TO_NET_32(bindAddr.getAddress());
	addr.port = port;

	return createServerInternal(addr, maxConnections);
}

bool ENETSocket::createClient(int maxConnections)
{
	if (m_pHost != 0)
	{
		setErrorString(ENETSOCKET_ERRSTR_ALREADYCREATED);
		return false;
	}

	ENetHost *pHost = enet_host_create(0, maxConnections, 0, 0);
	if (pHost == 0)
	{
		setErrorString(ENETSOCKET_ERRSTR_CANTCREATECLIENTHOST);
		return false;
	}

	struct sockaddr_in addr;

#if ! (defined (WIN32) || defined(_WIN32_WCE))
	memset(&addr, 0, sizeof(struct sockaddr_in));
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(0);
	addr.sin_port = htons(0);

	if (bind(pHost->socket, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) != 0)
	{
		enet_host_destroy(pHost);
		setErrorString(ENETSOCKET_ERRSTR_CANTBINDSOCKET);
		return false;
	}
#endif // ! (WIN32 || _WIN32_WCE)

	socklen_t addrLen = sizeof(struct sockaddr_in);
	memset(&addr, 0, addrLen);
	
	if (getsockname(pHost->socket, (struct sockaddr *)&addr, &addrLen) != 0)
	{
		enet_host_destroy(pHost);
		setErrorString(ENETSOCKET_ERRSTR_CANTGETLOCALPORT);
		return false;
	}
	
	m_localPort = ENET_NET_TO_HOST_16(addr.sin_port);
	m_pHost = pHost;
	m_server = false;
	m_nextConnID = 1;

	m_pConnectionIDs = new uint32_t[maxConnections];
	m_maxConnections = maxConnections;

	return true;
}

bool ENETSocket::close()
{
	if (m_pHost == 0)
	{
		setErrorString(ENETSOCKET_ERRSTR_NOTCREATED);
		return false;
	}

	enet_host_destroy(m_pHost);
	m_pHost = 0;
	m_server = false;
	m_localPort = 0;

	std::list<ENETPacket>::iterator it;
	
	for (it = m_packetQueue.begin() ; it != m_packetQueue.end() ; it++)
		enet_packet_destroy((*it).getPacket());

	m_packetQueue.clear();
	m_connectionMap.clear();

	return true;
}

bool ENETSocket::waitForEvent(int milliseconds)
{
	if (m_pHost == 0)
	{
		setErrorString(ENETSOCKET_ERRSTR_NOTCREATED);
		return false;
	}

	ENetEvent event;
	int status;

	if ((status = enet_host_service(m_pHost, &event, (uint32_t)milliseconds)) < 0)
	{
		setErrorString(ENETSOCKET_ERRSTR_ERRORINSERVICE);
		return false;
	}

	if (status == 0)
		return true;
	
	switch(event.type)
	{
	case ENET_EVENT_TYPE_CONNECT:
		{
			uint32_t connID = m_nextConnID;
	
			m_nextConnID++;
			if (m_nextConnID == 0)
				m_nextConnID = 1;
			
			ENetPeer *pPeer = event.peer;
			
			m_connectionMap[connID] = pPeer;
			pPeer->data = (void *)connID;
	
			IPv4Address srcAddress(pPeer->address.host, false);
			uint16_t srcPort = pPeer->address.port;

			buildNewConnectionArray();
	
			onNewConnection(connID, srcAddress, srcPort);
		}
		break;
	case ENET_EVENT_TYPE_DISCONNECT:
		{
			ENetPeer *pPeer = event.peer;
			
			uint32_t connID = (uint32_t)pPeer->data;
		
			std::map<uint32_t, ENetPeer *>::iterator it = m_connectionMap.find(connID);

			if (it != m_connectionMap.end())
				m_connectionMap.erase(it);

			buildNewConnectionArray();

			onCloseConnection(connID);
		}
		break;
	case ENET_EVENT_TYPE_RECEIVE:
		m_packetQueue.push_back(ENETPacket(event.packet, event.channelID, (uint32_t)event.peer->data));
		break;
	default:
		return true;
	}
	
	return true;
}

bool ENETSocket::poll()
{
	if (m_pHost == 0)
	{
		setErrorString(ENETSOCKET_ERRSTR_NOTCREATED);
		return false;
	}

	ENetEvent event;
	int status;
	
	do
	{
		if ((status = enet_host_service(m_pHost, &event, 0)) < 0)
		{
			setErrorString(ENETSOCKET_ERRSTR_ERRORINSERVICE);
			return false;
		}

		if (status != 0)
		{
			switch(event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				{
					uint32_t connID = m_nextConnID;
			
					m_nextConnID++;
					if (m_nextConnID == 0)
						m_nextConnID = 1;
					
					ENetPeer *pPeer = event.peer;
					
					m_connectionMap[connID] = pPeer;
					pPeer->data = (void *)connID;
			
					IPv4Address srcAddress(pPeer->address.host, false);
					uint16_t srcPort = pPeer->address.port;
		
					buildNewConnectionArray();
			
					onNewConnection(connID, srcAddress, srcPort);
				}
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				{
					ENetPeer *pPeer = event.peer;
					
					uint32_t connID = (uint32_t)pPeer->data;
				
					std::map<uint32_t, ENetPeer *>::iterator it = m_connectionMap.find(connID);
		
					if (it != m_connectionMap.end())
						m_connectionMap.erase(it);
		
					buildNewConnectionArray();
		
					onCloseConnection(connID);
				}
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				m_packetQueue.push_back(ENETPacket(event.packet, event.channelID, (uint32_t)event.peer->data));
				break;
			default:
				return true;
			}
		}
	} while (status > 0);
			
	return true;
}

bool ENETSocket::requestConnection(const IPv4Address &destAddress, uint16_t destPort, uint8_t channelCount)
{
	if (m_pHost == 0)
	{
		setErrorString(ENETSOCKET_ERRSTR_NOTCREATED);
		return false;
	}

	ENetAddress addr;

	addr.host = destAddress.getAddressNBO();
	addr.port = destPort;
	
	if (enet_host_connect(m_pHost, &addr, channelCount) == 0)
	{
		setErrorString(ENETSOCKET_ERRSTR_CANTCONNECT);
		return false;
	}

	enet_host_flush(m_pHost);
	
	return true;
}

bool ENETSocket::closeConnection(uint32_t connID)
{
	if (m_pHost == 0)
	{
		setErrorString(ENETSOCKET_ERRSTR_NOTCREATED);
		return false;
	}

	std::map<uint32_t, ENetPeer *>::iterator it = m_connectionMap.find(connID);

	if (it == m_connectionMap.end())
	{
		setErrorString(ENETSOCKET_ERRSTR_INVALIDCONNECTIONID);
		return false;
	}

#if 0
	enet_peer_disconnect((*it).second,0);
#else
	enet_peer_disconnect((*it).second);
#endif

	return true;
}

bool ENETSocket::getConnectionInfo(uint32_t connID, IPv4Address &addr, uint16_t &port)
{
	if (m_pHost == 0)
	{
		setErrorString(ENETSOCKET_ERRSTR_NOTCREATED);
		return false;
	}

	std::map<uint32_t, ENetPeer *>::iterator it = m_connectionMap.find(connID);

	if (it == m_connectionMap.end())
	{
		setErrorString(ENETSOCKET_ERRSTR_INVALIDCONNECTIONID);
		return false;
	}

	ENetPeer *pPeer = (*it).second;
	
	addr = IPv4Address(pPeer->address.host, false);
	port = pPeer->address.port;
	
	return true;
}

bool ENETSocket::write(const void *pData, size_t dataLength, bool reliable, uint8_t channel, uint32_t connID, bool allExceptConnID)
{
	if (m_pHost == 0)
	{
		setErrorString(ENETSOCKET_ERRSTR_NOTCREATED);
		return false;
	}

	uint32_t flags = 0;

	if (reliable)
		flags |= ENET_PACKET_FLAG_RELIABLE;
	
	ENetPacket *pPack = enet_packet_create(pData, dataLength, flags);
	
	std::map<uint32_t, ENetPeer *>::iterator it;
	for (it = m_connectionMap.begin() ; it != m_connectionMap.end() ; it++)
	{
		bool send = false;

		if (connID == 0 || connID == (*it).first)
			send = true;

		if (allExceptConnID)
		{
			if (send)
				send = false;
			else
				send = true;
		}

		if (send)
			enet_peer_send((*it).second, channel, pPack);
	}
	enet_host_flush(m_pHost);
	
	return true;
}

bool ENETSocket::getAvailableDataLength(size_t &length,bool &available)
{
	if (m_pHost == 0)
	{
		setErrorString(ENETSOCKET_ERRSTR_NOTCREATED);
		return false;
	}

	if (m_packetQueue.empty())
	{
		length = 0;
		available = false;
		return true;
	}

	length = (*(m_packetQueue.begin())).getPacket()->dataLength;
	available = true;
	
	return true;
}

bool ENETSocket::read(void *buffer, size_t &bufferSize, uint8_t *channel, uint32_t *connID)
{
	if (m_pHost == 0)
	{
		setErrorString(ENETSOCKET_ERRSTR_NOTCREATED);
		return false;
	}

	if (m_packetQueue.empty())
	{
		bufferSize = 0;
		setErrorString(ENETSOCKET_ERRSTR_NOPACKETAVAILABLE);
		return false;
	}

	size_t length;

	length = (*(m_packetQueue.begin())).getPacket()->dataLength;

	if (length > bufferSize)
	{
		bufferSize = length;
		setErrorString(ENETSOCKET_ERRSTR_BUFFERSIZETOOSMALL);
		return false;
	}

	ENETPacket enetPacket = *(m_packetQueue.begin());
	m_packetQueue.pop_front();

	if (channel)
		*channel = enetPacket.getChannel();
	if (connID)
		*connID = enetPacket.getConnectionID();

	ENetPacket *pPack = enetPacket.getPacket();

	memcpy(buffer, pPack->data, length);
	bufferSize = length;

	enet_packet_destroy(pPack);

	return true;
}

bool ENETSocket::createServerInternal(const ENetAddress &bindAddress, int maxConnections)
{
	if (m_pHost != 0)
	{
		setErrorString(ENETSOCKET_ERRSTR_ALREADYCREATED);
		return false;
	}
	
	ENetHost *pHost = enet_host_create(&bindAddress, maxConnections, 0, 0);
	if (pHost == 0)
	{
		setErrorString(ENETSOCKET_ERRSTR_CANTCREATESERVERHOST);
		return false;
	}

	m_pHost = pHost;
	m_server = true;
	m_localPort = bindAddress.port;
	m_nextConnID = 1;

	m_pConnectionIDs = new uint32_t[maxConnections];
	m_maxConnections = maxConnections;
	return true;
}

void ENETSocket::buildNewConnectionArray()
{
	size_t num = m_connectionMap.size();
	std::map<uint32_t, ENetPeer *>::iterator it = m_connectionMap.begin();
	
	for (size_t i = 0 ; i < num ; i++, it++)
		m_pConnectionIDs[i] = (*it).first;
}

} // end namespace

