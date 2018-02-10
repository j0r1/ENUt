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
 * \file multicasttunnelsocket.h
 */

#ifndef NUT_MULTICASTTUNNELSOCKET_H

#define NUT_MULTICASTTUNNELSOCKET_H

#include "nutconfig.h"

#ifdef NUTCONFIG_SUPPORT_ENET

#include "datagramsocket.h"
#include "enetsocket.h"
#include <list>

namespace nut
{

class IPv4Address;
class UDPSocket;
	
/** A socket to be used with the multicast tunnel server.
 *  A socket to be used with the multicast tunnel server. 
 *
 *  Multicasting happens entirely
 *  on the IP level, which implies that it is totally oblivious to the UDP port value.
 *  Now suppose that an on-line radiostation is multicasting its audio stream
 *  to a specific multicast group. All the UDP packets which are sent to the multicast 
 *  group will then contain the same destination port number. To capture the packets,
 *  a client will not only need to join the multicast group, but will also need an UDP
 *  socket which is bound to that particular UDP port. Since on a single host, only one
 *  socket can be bound to a specific port number, only one application will be able to
 *  capture the audio traffic on that host.
 *
 *  In some cases this can be a severe limitation. To work around this problem, a
 *  'multicast tunnel server' and a 'multicast tunnel socket' was created. The server
 *  is started on a specific host and receives commands from a MulticastTunnelSocket
 *  instance. It is the server which will join multicast groups and create UDP sockets
 *  on the appropriate port numbers, and the server will forward the incoming packets
 *  to all interested clients. Since only the server needs to use UDP sockets bound 
 *  on specific ports, the problem mentioned above is solved.
 *
 *  Currently, an ENet connection is used for the client-server communication.
 */
class ENUT_IMPORTEXPORT MulticastTunnelSocket : public DatagramSocket
{
public:
	/** Create an unnamed socket instance. */
	MulticastTunnelSocket();

	/** Create an instance with object name \c objName. */
	MulticastTunnelSocket(const std::string &objName);

	~MulticastTunnelSocket();
	
	/** Initialize the socket.
	 *  Initialize the socket.
	 *  \param serverAddr IP address of the multicast tunnel server applicatio to which we should connect.
	 *  \param serverPort Port number on which the server was started and will look for incoming connections.
	 *  \param mcastRemotePortNumber Incoming UDP packets which contain this destination port number and which
	 *                               arrive in a multicast group that we're interested in, will be forwarded to
	 *                               us.
	 */
	bool create(const IPv4Address &serverAddr, uint16_t serverPort, uint16_t mcastRemotePortNumber);

	/** Clean up the socket. */
	bool destroy();

	/** Return the port number to which the socket used for communication with the server is bound. */
	uint16_t getLocalPortNumber() const;

	bool joinMulticastGroup(const NetworkLayerAddress &groupAddress);
	bool leaveMulticastGroup(const NetworkLayerAddress &groupAddress);

	bool write(const void *pData, size_t &length, const NetworkLayerAddress &destinationAddress, uint16_t destinationPort);
	
	bool getAvailableDataLength(size_t &length, bool &available);
	bool read(void *buffer, size_t &bufferSize);

	const NetworkLayerAddress *getLastSourceAddress() const;
	uint16_t getLastSourcePort() const;
	const NetworkLayerAddress *getLastDestinationAddress() const;
protected:
	ENetSocket getSocketDescriptor()								{ return m_pEnetSocket->getSocketDescriptor(); }
private:
	// Make sure we can't copy the socket
	MulticastTunnelSocket(const MulticastTunnelSocket &s) : DatagramSocket(s)			{ }
	MulticastTunnelSocket &operator=(const MulticastTunnelSocket &s)				{ }
	
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

#endif // NUTCONFIG_SUPPORT_ENET

#endif // NUT_MULTICASTTUNNELSOCKET_H
