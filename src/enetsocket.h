/*
    
  This file is a part of ENUt, a library containing network
  programming utilities.
  
  Copyright (C) 2006-2012  Hasselt University - Expertise Centre for
                      Digital Media (EDM) (http://www.edm.uhasselt.be)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  
  USA

*/

/**
 * \file enetsocket.h
 */

#ifndef NUT_ENETSOCKET_H

#define NUT_ENETSOCKET_H

#include "nutconfig.h"

#ifdef NUTCONFIG_SUPPORT_ENET

#include "socket.h"
#include "ipv4address.h"
#include <enet/enet.h>
#include <map>
#include <list>

namespace nut
{

/** An ENet based socket. 
 *  This socket works according to the ENet protocol. Like UDP, it is
 *  a packet-based protocol, but unlike UDP it allows packets to
 *  be sent reliably. See http://enet.bespin.org/ for more information
 *  about the ENet protocol.
 */
class ENUT_IMPORTEXPORT ENETSocket : public Socket
{
public:
	/** Creates an ENet socket. */
	ENETSocket();

	/** Creates an ENet socket with object name \c objName. */
	ENETSocket(const std::string &objName);
	
	~ENETSocket();

	/** Initializes the ENet library. */
	static bool ENETStartup();

	/** De-initializes the ENet library. */
	static void ENETCleanup();

	/** Creates an ENet socket in server mode.
	 *  Creates an ENet socket in server mode. This means that the socket
	 *  can accept incoming connections as well as outgoing connections.
	 *  \param maxConnections The maximum number of connections allowed (both incoming and outgoing).
	 *  \param port If not zero, the socket will be bound on a specific port.
	 */
	bool createServer(int maxConnections, uint16_t port = 0);

	/** Creates an ENet socket in server mode.
	 *  Creates an ENet socket in server mode. This means that the socket
	 *  can accept incoming connections as well as outgoing connections.
	 *  \param bindAddr The IP address to bind the socket to.
	 *  \param maxConnections The maximum number of connections allowed (both incoming and outgoing).
	 *  \param port If not zero, the socket will be bound on a specific port.
	 */
	bool createServer(const IPv4Address &bindAddr, int maxConnections, uint16_t port = 0);

	/** Creates an ENet socket in client mode.
	 *  Creates an ENet socket in client mode. This means that it can create outgoing
	 *  connections, but it cannot accept incoming ones.
	 *  \param maxConnections The maximum number of outgoing connections that can be established.
	 */
	bool createClient(int maxConnections);

	/** Returns a flag indicating if the socket was created in server mode or client mode. */
	bool isServer() const										{ return m_server; }

	/** Closes the socket and all associated connections. */
	bool close();
	
	/** Returns the port number that the socket is bound to. */
	uint16_t getLocalPort() const									{ return m_localPort; }

	/** Waits at most the specified time for an event to occur. 
	 *  Waits at most the specified time for an event to occur. The first event
	 *  that occurs is processed as well. An event can be a client that wants
	 *  to connect, data that arrives etc.
	 *  \param milliseconds The maximum amount of time to wait (in milliseconds)
	 */
	bool waitForEvent(int milliseconds);

	/** Process all currently available events. 
	 *  Process all currently available events. An event can be a client that wants
	 *  to connect, data that arrives etc.
	 */
	bool poll();
	
	/** Try to establish a connection to the specified server.
	 *  Try to establish a connection to the specified server.
	 *  \param destAddress IP address of the server.
	 *  \param destPort Port number on which the server is listening for incoming connections.
	 *  \param channelCount The number of communication channels that should be allocated.
	 */
	bool requestConnection(const IPv4Address &destAddress, uint16_t destPort, uint8_t channelCount);

	/** Close the connection specified by \c connId. */
	bool closeConnection(uint32_t connID);

	/** Returns the current number of established connections. */
	int getNumberOfConnections() const								{ return (int)m_connectionMap.size(); }

	/** Returns a pointer to the connection identifiers of the currently established connections. */
	const uint32_t *getConnectionIDs() const							{ return m_pConnectionIDs; }

	/** Obtain additional information about a specific connection.
	 *  Obtain additional information about a specific connection.
	 *  \param connID The connection for which additional information is requested.
	 *  \param addr In this variable, the IP address of the end-point of this connection will be stored.
	 *  \param port In this variable, the port number of the end-point of this connection will be stored.
	 */
	bool getConnectionInfo(uint32_t connID, IPv4Address &addr, uint16_t &port) const;
	
	/** Writes data on a connection or set of connections.
	 *  Writes data on a connection or set of connections. If
	 *  \param pData Pointer to the data which should be sent.
	 *  \param dataLength Number of bytes which should be sent.
	 *  \param reliable Flag indicating if the data should be sent reliably.
	 *  \param channel The channel number on which the data should be sent.
	 *  \param connID The connection on which the data should be sent. A value
	 *                of zero specifies all existing connections.
	 *  \param allExceptConnID If this flag is true, the data will be sent over
	 *                         all connections except the one specified in \c connID.
	 */
	bool write(const void *pData, size_t dataLength, bool reliable = false, uint8_t channel = 0, uint32_t connID = 0, bool allExceptConnID = false);

	/** Obtain information about the data which can be read.
	 *  Obtain information about the data which can be read.
	 *  \param length Will contain the number of bytes of the first packet that can be read
	 *                (can be zero!)
	 *  \param available Is set to true if a packet is available, to false otherwise.
	 */
	bool getAvailableDataLength(size_t &length, bool &available);

	/** Reads data from the socket.
	 *  Reads data from the socket.
	 *  \param buffer A pointer to the buffer in which the data can be stored.
	 *  \param bufferSize Initially this must be set to the maximum number of bytes
	 *                    that can be stored in \c buffer. The value will be adjusted
	 *                    to match the actual number of bytes in the first available 
	 *                    packet. If \c bufferSize is smaller than the actual number
	 *                    of bytes in the first packet, the function will fail.
	 *  \param channel If not null, the channel number on which the data was received
	 *                 will be stored here.
	 *  \param connID If not null, the identifier of the connection on which the data
	 *                was received will be stored here.
	 */
	bool read(void *buffer, size_t &bufferSize, uint8_t *channel = 0, uint32_t *connID = 0);
protected:
	/** Override this function to get notified when a new incoming connection is detected
	 *  or when an outgoing connection is successfully established.
	 *  Override this function to get notified when a new incoming connection is detected
	 *  or when an outgoing connection is successfully established.
	 *  \param connID The identifier of the new connection.
	 *  \param sourceAddress IP address of the end-point of this connection.
	 *  \param sourcePort Port number of the end-point of this connection.
	 */
	virtual void onNewConnection(uint32_t connID, const IPv4Address &sourceAddress, uint16_t sourcePort)	{ }

	/** Override this function to get notified when a connection is closed.
	 *  Override this function to get notified when a connection is closed.
	 *  \param connID Identifier of the connection that was closed.
	 */
	virtual void onCloseConnection(uint32_t connID)								{ }
private:
	// make sure we can't copy the socket
	ENETSocket(const ENETSocket &s) : Socket(s)							{ }
	ENETSocket &operator=(const ENETSocket &s)							{ }
	
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

protected: 
	// Should probably only be used by the socket waiter
	ENetSocket getSocketDescriptor()								{ return m_pHost->socket; }
};

} // end namespace

#endif // NUTCONFIG_SUPPORT_ENET

#endif // NUT_ENETSOCKET_H

