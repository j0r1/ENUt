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
 * \file tcpv4socket.h
 */

#ifndef NUT_TCPV4SOCKET_H

#define NUT_TCPV4SOCKET_H

#include "nutconfig.h"
#include "tcpsocket.h"
#include "ipv4address.h"

namespace nut
{

/** A TCP over IPv4 socket. */
class ENUT_IMPORTEXPORT TCPv4Socket : public TCPSocket
{
public:
	/** Construct a TCP over IPv4 socket. */
	TCPv4Socket();

	/** Construct a TCP over IPv4 socket with object name \c objName. */
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
	
	const NetworkLayerAddress *getDestinationAddress() const				{ return m_destIP; }
	uint16_t getDestinationPort() const							{ return m_destPort; }
	
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
	// make sure we can't copy the socket
	TCPv4Socket(const TCPv4Socket &s) : TCPSocket(s)					{ }
	TCPv4Socket &operator=(const TCPv4Socket &s)						{ return *this; }
	
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
