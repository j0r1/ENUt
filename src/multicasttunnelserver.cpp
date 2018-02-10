#include "multicasttunnelserver.h"
#include "enetsocket.h"
#include "udpv4socket.h"
#include "socketwaiter.h"
#include <stdio.h>
#include <signal.h>
#include <string>
#include <iostream>
#include <list>
#include <map>
#include <stdarg.h>
#include <time.h>

using namespace nut;

int debugLevel = 0;

#define LOG_DEBUG	3
#define LOG_INFO	2
#define LOG_ERROR	1

#define MAXMSGLEN	1024

void writeLog(int level, const char *fmt, ...)
{
	if (level > debugLevel)
		return;

	static char str[MAXMSGLEN];
	
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(str,MAXMSGLEN,fmt,ap);
	va_end(ap);
	str[MAXMSGLEN-1] = 0;

	time_t t = time(0);
	struct tm *lt = localtime(&t);

	printf("[%02d:%02d:%02d %04d/%02d/%02d] %s\n", lt->tm_hour, lt->tm_min, lt->tm_sec, 
			                               1900+lt->tm_year, lt->tm_mon, lt->tm_mday,
						       str);
}

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

bool stopLoop = false;

#if ! (defined(WIN32) || defined(_WIN32_WCE))
void SignalHandler(int val)
{
	stopLoop = true;
}
#endif // ! (WIN32 || _WIN32_WCE)

inline void checkError(bool ret, ErrorBase &obj)
{
	if (ret)
		return;

	writeLog(LOG_ERROR,"Error in: %s", obj.getObjectName().c_str());
	writeLog(LOG_ERROR,"Description: %s", obj.getErrorString().c_str());
	
	exit(-1);
}

inline void checkError(bool ret, ErrorBase *obj)
{
	if (ret)
		return;

	writeLog(LOG_ERROR,"Error in: %s", obj->getObjectName().c_str());
	writeLog(LOG_ERROR,"Description: %s", obj->getErrorString().c_str());
	
	exit(-1);
}

class ClientInfo;
class MulticastSocketInfo;

std::map<uint16_t, MulticastSocketInfo *> socketInfo; // map of port versus socket info
std::map<uint32_t, ClientInfo *> connectedClients;

class MulticastSocketInfo
{
public:
	MulticastSocketInfo(UDPv4Socket *pSock, uint16_t port)
	{	
		writeLog(LOG_DEBUG, "Created multicast socket for port %d",(int)port);
		m_pSock = pSock;
		m_port = port;
	}

	~MulticastSocketInfo()
	{
		delete m_pSock;
		writeLog(LOG_DEBUG, "Destroyed multicast socket for port %d",(int)m_port);
	}

	UDPv4Socket *getSocket()								{ return m_pSock; }

	void distributeData(void *pBuffer, size_t length, uint32_t mcastIP, ENETSocket &serverSocket);
	void registerClient(ClientInfo *pInf)							{ m_registeredClients.push_back(pInf); }
	void unregisterClient(ClientInfo *pInf);
	void subscribe(uint32_t ip, ClientInfo *pInf);
	void unsubscribe(uint32_t ip, ClientInfo *pInf);
	size_t getNumberOfClients() const							{ return m_registeredClients.size(); }
	uint16_t getPort() const								{ return m_port; }
private:
	std::list<ClientInfo *> m_registeredClients;
	std::map<uint32_t, std::list<ClientInfo *> *> m_groupSubscriptions;
	
	UDPv4Socket *m_pSock;
	uint16_t m_port;
};

class ClientInfo
{
public:
	ClientInfo(uint32_t connID)
	{
		writeLog(LOG_DEBUG, "Created client for connection %d",(int)connID);
		m_connID = connID;
		m_pMcastSocket = 0;
		m_started = false;
	}

	~ClientInfo()
	{	
		if (m_pMcastSocket == 0) // client is not registered with any multicast socket yet
			return; 
		
		std::list<uint32_t>::const_iterator it;

		for (it = m_subscribedGroups.begin() ; it != m_subscribedGroups.end() ; it++)
			m_pMcastSocket->unsubscribe((*it),this);
		
		m_pMcastSocket->unregisterClient(this);
		if (m_pMcastSocket->getNumberOfClients() == 0) // no other clients interested in this socket, clean up
		{
			std::map<uint16_t, MulticastSocketInfo *>::iterator it = socketInfo.begin();
			bool found = false;

			while (!found && it != socketInfo.end())
			{
				if ((*it).second == m_pMcastSocket)
					found = true;
				else
					it++;
			}

			if (!found)
				writeLog(LOG_INFO, "WARNING: Couldn't find multicast socket info for port %d in list",(int)(*it).first);
			else
			{
				delete (*it).second;
				socketInfo.erase(it);
			}
		}

		writeLog(LOG_DEBUG, "Destroyed client for connection %d",(int)m_connID);
	}

	uint32_t getConnectionID() const								{ return m_connID; }

	void writeData(const void *pBuf, size_t length, const IPv4Address &destIP, uint16_t destPort)
	{
		if (m_pMcastSocket == 0)
		{
			writeLog(LOG_INFO, "WARNING: Received data on connection %d, but no multicast socket is set",(int)m_connID);
			return;
		}

		size_t l = length;
		m_pMcastSocket->getSocket()->write(pBuf, l, destIP, destPort);
	}

	void setStarted()										{ m_started = true; }
	bool isStarted() const										{ return m_started; }

	bool openPort(uint16_t portNum)
	{
		writeLog(LOG_DEBUG, "Connection %d requested to open port %d",(int)m_connID,(int)portNum);
		
		if (m_pMcastSocket != 0)
		{
			writeLog(LOG_INFO, "WARNING: Client %d already has a socket",(int)m_connID);
			if (portNum == m_pMcastSocket->getPort())
				return true;
			return false;
		}

		std::map<uint16_t, MulticastSocketInfo *>::iterator it = socketInfo.find(portNum);

		if (it != socketInfo.end())
		{
			writeLog(LOG_DEBUG, "Found existing multicast socket on port %d; using it",(int)portNum);
			m_pMcastSocket = (*it).second;
			m_pMcastSocket->registerClient(this);
			return true;
		}

		writeLog(LOG_DEBUG, "Didn't find existing multicast socket on port %d; creating new one",(int)portNum);

		// no socket open for this port yet, try to open one

		UDPv4Socket *pSock = new UDPv4Socket();

		if (!pSock->create(portNum, true))
		{
			writeLog(LOG_INFO, "WARNING: Couldn't open socket on port %d: %s",(int)portNum,pSock->getErrorString().c_str());
			delete pSock;
			return false;
		}

		// enable multicast loopback

		bool ret;

#ifdef WIN32
		DWORD val = 1;
		ret = pSock->setSocketOption(IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&val, sizeof(DWORD));
#else
		int val = 1;
		ret = pSock->setSocketOption(IPPROTO_IP, IP_MULTICAST_LOOP, &val, sizeof(int));
#endif // WIN32
		if (!ret)
		{
			writeLog(LOG_INFO, "WARNING: Couldn't set IP_MULTICAST_LOOP option for socket on port %d: %s",(int)portNum,pSock->getErrorString().c_str());
			delete pSock;
			return false;
		}

		// ok, socket created successfully

		MulticastSocketInfo *pSockInf = new MulticastSocketInfo(pSock, portNum);
		
		m_pMcastSocket = pSockInf;
		socketInfo[portNum] = pSockInf;
		m_pMcastSocket->registerClient(this);
		
		return true;
	}
	
	void joinGroup(uint32_t ip)
	{
		writeLog(LOG_DEBUG, "Client on connection %d requested to join %s", (int)m_connID,IPv4Address(ip).getAddressString().c_str());
		
		if (!m_pMcastSocket)
		{
			writeLog(LOG_INFO, "WARNING: Connection %d wants to join a group but doesn't have a multicast socket", (int)m_connID);
			return;
		}
		
		std::list<uint32_t>::iterator it;
		bool found = false;
	
		for (it = m_subscribedGroups.begin() ; !found && it != m_subscribedGroups.end() ; it++)
		{
			if ((*it) == ip)
				found = true;
		}

		if (found)
			writeLog(LOG_INFO, "WARNING: Connection %d already subscribed to %s", (int)m_connID,IPv4Address(ip).getAddressString().c_str());
		else
		{
			m_pMcastSocket->subscribe(ip,this);
			m_subscribedGroups.push_back(ip);
		}
	}

	void leaveGroup(uint32_t ip)
	{
		writeLog(LOG_DEBUG, "Client on connection %d requested to leave %s", (int)m_connID,IPv4Address(ip).getAddressString().c_str());

		if (!m_pMcastSocket)
		{
			writeLog(LOG_INFO, "WARNING: Connection %d wants to join a group but doesn't have a multicast socket", (int)m_connID);
			return;
		}
		
		std::list<uint32_t>::iterator it;
		bool found = false;
	
		it = m_subscribedGroups.begin(); 
		while (!found && it != m_subscribedGroups.end())
		{
			if ((*it) == ip)
				found = true;
			else
				it++;
		}

		if (!found)
			writeLog(LOG_INFO, "WARNING: Connection %d not subscribed to %s", (int)m_connID,IPv4Address(ip).getAddressString().c_str());
		else
		{
			m_pMcastSocket->unsubscribe(ip,this);
			m_subscribedGroups.erase(it);
		}	
	}
private:
	uint32_t m_connID;
	MulticastSocketInfo *m_pMcastSocket;
	std::list<uint32_t> m_subscribedGroups;
	bool m_started;
};

void MulticastSocketInfo::distributeData(void *pBuffer, size_t length, uint32_t mcastIP, ENETSocket &serverSocket)
{
	std::map<uint32_t, std::list<ClientInfo *> *>::iterator it = m_groupSubscriptions.find(mcastIP);

	if (it == m_groupSubscriptions.end())
		return;
	
	std::list<ClientInfo *> *pClients = (*it).second;
	std::list<ClientInfo *>::iterator it2;

	for (it2 = pClients->begin() ; it2 != pClients->end() ; it2++)
	{
		if ((*it2)->isStarted())
			serverSocket.write(pBuffer,length,false,MCASTTUNNEL_CHANNEL_DATA,(*it2)->getConnectionID(),false);
	}
}

void MulticastSocketInfo::unregisterClient(ClientInfo *pInf)
{
	std::list<ClientInfo *>::iterator it = m_registeredClients.begin();
	bool found = false;

	while (!found && it != m_registeredClients.end())
	{
		if ((*it) == pInf)
			found = true;
		else
			it++;
	}
	if (!found)
		writeLog(LOG_INFO, "WARNING: Client with ID %d was not found in registered clients list",(int)pInf->getConnectionID());
	else
		m_registeredClients.erase(it);
}

void MulticastSocketInfo::subscribe(uint32_t ip, ClientInfo *pInf)
{
	std::map<uint32_t, std::list<ClientInfo *> *>::iterator it = m_groupSubscriptions.find(ip);
	IPv4Address addr(ip);
	
	if (it == m_groupSubscriptions.end())
	{
		std::list<ClientInfo *> *pList = new std::list<ClientInfo *>;
		
		pList->push_back(pInf);
		m_pSock->joinMulticastGroup(addr);
		m_groupSubscriptions[ip] = pList;
		return;
	}
	
	// socket already joined group
	
	std::list<ClientInfo *> *pList = (*it).second;
	pList->push_back(pInf);
}

void MulticastSocketInfo::unsubscribe(uint32_t ip, ClientInfo *pInf)
{
	std::map<uint32_t, std::list<ClientInfo *> *>::iterator it = m_groupSubscriptions.find(ip);
	IPv4Address addr(ip);
	
	if (it == m_groupSubscriptions.end())
	{
		writeLog(LOG_INFO, "WARNING: Client with ID %d does not have a subscription for %s (IP not found)",(int)pInf->getConnectionID(),addr.getAddressString().c_str());
		return;
	}

	std::list<ClientInfo *> *pClients = (*it).second;
	std::list<ClientInfo *>::iterator it2 = pClients->begin();
	bool found = false;
	
	while (!found && it2 != pClients->end())
	{
		if ((*it2) == pInf)
			found = true;
		else
			it2++;
	}

	if (!found)
		writeLog(LOG_INFO, "WARNING: Client with ID %d does not have a subscription for %s (ClientInfo not found)",(int)pInf->getConnectionID(),addr.getAddressString().c_str());
	else
	{
		pClients->erase(it2);

		// if no clients are interested in this multicast group anymore, remove the entry and
		// leave the multicast group
		if (pClients->empty()) 
		{
			delete pClients;
			m_groupSubscriptions.erase(it);

			m_pSock->leaveMulticastGroup(addr);
		}
	}
}

class ServerENETSocket : public ENETSocket
{
public:
	ServerENETSocket(const std::string &name) : ENETSocket(name) { }
	~ServerENETSocket()						  { }
protected:
	void onNewConnection(uint32_t connID, const IPv4Address &sourceAddress, uint16_t sourcePort)
	{
		writeLog(LOG_INFO, "Accepted connection from %s:%d, connection ID %d", sourceAddress.getAddressString().c_str(), (int)sourcePort, connID);

		connectedClients[connID] = new ClientInfo(connID);
	}
	
	void onCloseConnection(uint32_t connID)
	{
		std::map<uint32_t, ClientInfo *>::iterator it = connectedClients.find(connID);
		bool found = false;

		if (it == connectedClients.end())
			writeLog(LOG_INFO, "WARNING: Connection ID %d not found in connectedClients list", connID);
		else
		{
			delete (*it).second; // destructor handles unregistration of multicast groups etc
			connectedClients.erase(it);
		}

		writeLog(LOG_INFO, "Removed connection ID %d", connID);
	}
};

void processIncomingPackets(ENETSocket &serverSocket)
{
	size_t len = 0;
	bool avail = 0;

	serverSocket.getAvailableDataLength(len,avail);

	while (avail)
	{
		uint8_t *pBuffer = new uint8_t [len+1];
		uint8_t channel;
		uint32_t connID;

		serverSocket.read(pBuffer, len, &channel, &connID);
		if (channel == MCASTTUNNEL_CHANNEL_CONTROL)
		{
			if (len < 1)
				writeLog(LOG_INFO, "WARNING: Received control data with invalid length %d (connection ID %d)",(int)len,(int)connID);
			else
			{
				std::map<uint32_t, ClientInfo *>::iterator it = connectedClients.find(connID);

				if (it == connectedClients.end())
					writeLog(LOG_INFO, "WARNING: No ClientInfo found for connection ID %d",(int)connID);
				else
				{
					ClientInfo *pClient = (*it).second;
						
					
					switch(pBuffer[0])
					{
					case MCASTTUNNEL_COMMAND_PORT:
						if (len != 1+sizeof(uint16_t))
							writeLog(LOG_INFO, "WARNING: Invalid port command of length %d for connection ID %d",(int)len,(int)connID);
						else
						{
							uint16_t port = readUint16(pBuffer+1);
							uint8_t buf[1];
							
							if (pClient->openPort(port))
								buf[0] = MCASTTUNNEL_COMMAND_ACK;
							else
								buf[0] = MCASTTUNNEL_COMMAND_ACKERR;
							
							serverSocket.write(buf, 1, true, MCASTTUNNEL_CHANNEL_CONTROL, connID, false);
						}
						break;
					case MCASTTUNNEL_COMMAND_START:
						if (len != 1)
							writeLog(LOG_INFO, "WARNING: Invalid start command of length %d for connection ID %d",(int)len,(int)connID);
						else
							pClient->setStarted();
						break;
					case MCASTTUNNEL_COMMAND_JOIN:
						if (len != 1+sizeof(uint32_t))
							writeLog(LOG_INFO, "WARNING: Invalid join command of length %d for connection ID %d",(int)len,(int)connID);
						else
						{
							uint32_t ip = readUint32(pBuffer+1);

							pClient->joinGroup(ip);
						}
						break;
					case MCASTTUNNEL_COMMAND_LEAVE:
						if (len != 1+sizeof(uint32_t))
							writeLog(LOG_INFO, "WARNING: Invalid leave command of length %d for connection ID %d",(int)len,(int)connID);
						else
						{
							uint32_t ip = readUint32(pBuffer+1);

							pClient->leaveGroup(ip);
						}
						break;
					default:
						writeLog(LOG_INFO, "WARNING: Received invalid command %d (connection ID %d)",(int)pBuffer[0],(int)connID);
					}
				}
			}
				
		}
		else if (channel == MCASTTUNNEL_CHANNEL_DATA)
		{
			if (len < sizeof(uint32_t)+sizeof(uint16_t))
				writeLog(LOG_INFO, "WARNING: Received data packet with invalid length %d (connection ID %d)",(int)len,(int)connID);
			else
			{
				uint32_t destIP = readUint32(pBuffer);
				uint16_t destPort = readUint16(pBuffer+sizeof(uint32_t));

				// look up connection
				
				std::map<uint32_t, ClientInfo *>::iterator it = connectedClients.find(connID);

				if (it == connectedClients.end())
					writeLog(LOG_INFO, "WARNING: Connection ID %d not found in connectedClients",(int)connID);
				else
				{
					ClientInfo *pInf = (*it).second;

					pInf->writeData(pBuffer+sizeof(uint32_t)+sizeof(uint16_t),len-sizeof(uint16_t)-sizeof(uint32_t),IPv4Address(destIP),destPort);
				}
			}
		}
		else
			writeLog(LOG_INFO, "WARNING: Received data on invalid channel %d (connection ID %d)",(int)channel,(int)connID);

		delete [] pBuffer;
		
		len = 0;
		avail = false;
		serverSocket.getAvailableDataLength(len,avail);
	}
}

void processIncomingPackets(MulticastSocketInfo &sockInf, ENETSocket &serverSocket)
{
	UDPv4Socket *pSock = sockInf.getSocket();

	size_t len = 0;
	bool avail = false;

	pSock->getAvailableDataLength(len,avail);

	while (avail)
	{
		uint8_t *pBuf = new uint8_t[len+sizeof(uint32_t)*2+sizeof(uint16_t)];

		pSock->read(pBuf+sizeof(uint32_t)*2+sizeof(uint16_t),len);

		const IPv4Address *pSrcAddr = (const IPv4Address *)pSock->getLastSourceAddress();
		const IPv4Address *pDstAddr = (const IPv4Address *)pSock->getLastDestinationAddress(); // multicast address to which the packet was sent
		uint16_t srcPort = pSock->getLastSourcePort();
		
		writeUint32(pBuf,pSrcAddr->getAddress());
		writeUint16(pBuf+sizeof(uint32_t),srcPort);
		writeUint32(pBuf+sizeof(uint32_t)+sizeof(uint16_t),pDstAddr->getAddress());
		
		sockInf.distributeData(pBuf,len+sizeof(uint32_t)*2+sizeof(uint16_t),pDstAddr->getAddress(),serverSocket);

		delete [] pBuf;

		len = 0;
		avail = false;
		pSock->getAvailableDataLength(len,avail);
	}
}

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		std::cerr << "Usage: " << std::endl;
		std::cerr << " " << std::string(argv[0]) << " port maxconnections debuglevel" << std::endl;
		return -1;
	}

	debugLevel = atoi(argv[3]);

#if defined(WIN32) || defined(_WIN32_WCE)
	WSADATA dat;
	WSAStartup(MAKEWORD(2,2),&dat);
#else	
	signal(SIGTERM,SignalHandler);
	signal(SIGHUP,SignalHandler);
	signal(SIGQUIT,SignalHandler);
	signal(SIGABRT,SignalHandler);
	signal(SIGINT,SignalHandler);
#endif // WIN32 || _WIN32_WCE	
	ENETSocket::ENETStartup();
	
	ServerENETSocket serverSocket("ServerSocket");
	int maxConn = atoi(argv[2]);
	bool ret;
	
	ret = serverSocket.createServer(atoi(argv[2]),(uint16_t)atoi(argv[1]));
	checkError(ret, serverSocket);

	writeLog(LOG_INFO,"Server was started on port %d using maximum number of connections %d",(int)serverSocket.getLocalPort(), maxConn);

	while (!stopLoop)
	{
		SocketWaiter waiter;
		std::map<uint16_t, MulticastSocketInfo *>::iterator it;
		
		waiter.addSocket(serverSocket);
		
		for (it = socketInfo.begin() ; it != socketInfo.end() ; it++)
			waiter.addSocket(*((*it).second->getSocket()));

		ret = waiter.wait(0,100000); // wait at most 100 milliseconds
		checkError(ret, waiter);

		ret = serverSocket.poll();
		checkError(ret, serverSocket);

		processIncomingPackets(serverSocket);

		for (it = socketInfo.begin() ; it != socketInfo.end() ; it++)
			processIncomingPackets(*((*it).second),serverSocket);
	}
	
	writeLog(LOG_INFO, "Shutting down");
	
	serverSocket.close();
	ENETSocket::ENETCleanup();

#if defined(WIN32) || defined(_WIN32_WCE)
	WSACleanup();
#endif // WIN32 || _WIN32_WCE

	return 0;
}

