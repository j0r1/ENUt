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
 * \file tcpv6socket.h
 */

#ifndef NUT_TCPV6SOCKET_H

#define NUT_TCPV6SOCKET_H

#include "nutconfig.h"

#ifdef NUTCONFIG_SUPPORT_IPV6

#include "tcpsocket.h"
#include "ipv6address.h"

namespace nut
{

/** A TCP over IPv6 socket. */
class ENUT_IMPORTEXPORT TCPv6Socket : public TCPSocket
{
public:
	/** Construct a TCP over IPv6 socket. */
	TCPv6Socket();

	/** Construct a TCP over IPv6 socket with object name \c objName. */
	TCPv6Socket(const std::string &objName);

	~TCPv6Socket();

	bool create(uint16_t portNumber = 0);
	bool create(const NetworkLayerAddress &bindAddress, uint16_t portNumber = 0);
	bool destroy();

	bool setNonBlocking(bool f = true);
	
	uint16_t getLocalPortNumber()								{ return m_localPort; }

	bool connect(const NetworkLayerAddress &hostAddress, uint16_t portNumber);
	bool listen(int backlog);
	bool accept(TCPSocket **newsock);
	bool isConnected()									{ return m_connected; }
	
	const NetworkLayerAddress *getDestinationAddress() const				{ return m_destIP; }
	uint16_t getDestinationPort() const							{ return m_destPort; }
	
	bool write(const void *data, size_t &length);
	bool getAvailableDataLength(size_t &length);
	bool read(void *buffer, size_t &bufferSize);
protected:
	int getSocketDescriptor()								{ return m_sock; }
private:
	// make sure we can't copy the socket
	TCPv6Socket(const TCPv6Socket &s) : TCPSocket(s)					{ }
	TCPv6Socket &operator=(const TCPv6Socket &s)						{ }
	
	TCPv6Socket(int sock, uint16_t localPort, in6_addr destIP, uint16_t destPort);
	bool internalCreate(in6_addr ip, uint16_t port);
	void zeroAll();

	int m_sock;
	bool m_connected, m_listening;
	uint16_t m_localPort;
	uint16_t m_destPort;
	IPv6Address *m_destIP;
};

}

#endif // NUTCONFIG_SUPPORT_IPV6

#endif // NUT_TCPV6SOCKET_H

