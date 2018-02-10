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
 * \file udpv4socket.h
 */

#ifndef NUT_UDPV4SOCKET_H

#define NUT_UDPV4SOCKET_H

#include "nutconfig.h"
#include "udpsocket.h"
#include "ipv4address.h"

#if defined(WIN32) && !defined(_WIN32_WCE)
	#include <mswsock.h>
#endif // WIN32

namespace nut
{

/** An UDP over IPv4 socket. */
class ENUT_IMPORTEXPORT UDPv4Socket : public UDPSocket
{
public:
	/** Constructs an UDP over IPv4 socket. */
	UDPv4Socket();

	/** Constructs an UDP over IPv4 socket with object name \c objName. */
	UDPv4Socket(const std::string &objName);
	
	~UDPv4Socket();

	bool create(uint16_t portNumber = 0, bool obtainDestination = false);
	bool create(NetworkLayerAddress &bindAddress, uint16_t portNumber = 0, bool obtainDestination = false);
	bool destroy();
	
	bool setNonBlocking(bool f = true);
	
	uint16_t getLocalPortNumber() const								{ return m_localPort; }
	
	bool joinMulticastGroup(const NetworkLayerAddress &groupAddress);
	bool leaveMulticastGroup(const NetworkLayerAddress &groupAddress);
	bool setMulticastTTL(uint8_t ttl);
	
	bool write(const void *data, size_t &length, const NetworkLayerAddress &destinationAddress, 
	           uint16_t destinationPort);
	bool getAvailableDataLength(size_t &length, bool &available);
	bool read(void *buffer, size_t &bufferSize);
	const NetworkLayerAddress *getLastSourceAddress() const						{ return &m_srcIP; }
	uint16_t getLastSourcePort() const								{ return m_srcPort; }
	const NetworkLayerAddress *getLastDestinationAddress() const					{ return &m_dstIP; }
protected:
#if (defined(WIN32) || defined(_WIN32_WCE))
	SOCKET getSocketDescriptor()									{ return m_sock; }
#else	
	int getSocketDescriptor()									{ return m_sock; }
#endif // WIN32 || _WIN32_WCE
private:
	// make sure we can't copy the socket
	UDPv4Socket(const UDPv4Socket &s) : UDPSocket(s)						{ }
	UDPv4Socket &operator=(const UDPv4Socket &s)							{ return *this; }
	
	void zeroAll();
	bool internalCreate(uint32_t ip, uint16_t port, bool obtainDestination);
	std::string getSocketErrorString();

#if (defined(WIN32) || defined(_WIN32_WCE))
	SOCKET m_sock;

#ifndef _WIN32_WCE
	LPFN_WSARECVMSG WSARecvMsg;
#endif // _WIN32_WCE
#else
	int m_sock;
#endif // WIN32 || _WIN32_WCE
	
	bool m_isBlocking;
	uint16_t m_localPort;
	uint32_t m_bindIP;
	IPv4Address m_srcIP;
	uint16_t m_srcPort;
	IPv4Address m_dstIP;
	bool m_obtainDest;
};

} // end namespace

#endif // NUT_UDPV4SOCKET_H

