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
 * \file udpv6socket.h
 */

#ifndef NUT_UDPV6SOCKET_H

#define NUT_UDPV6SOCKET_H

#include "nutconfig.h"

#ifdef NUTCONFIG_SUPPORT_IPV6

#include "udpsocket.h"
#include "ipv6address.h"
#include <netinet/in.h>

namespace nut
{

/** An UDP over IPv6 socket. */
class ENUT_IMPORTEXPORT UDPv6Socket : public UDPSocket
{
public:
	/** Creates an UDP over IPv6 socket. */
	UDPv6Socket();

	/** Creates an UDP over IPv6 socket with object name \c objName. */
	UDPv6Socket(const std::string &objName);

	~UDPv6Socket();

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
	int getSocketDescriptor()									{ return m_sock; }
private:
	// make sure we can't copy the socket
	UDPv6Socket(const UDPv6Socket &s) : UDPSocket(s)						{ }
	UDPv6Socket &operator=(const UDPv6Socket &s)							{ }
	
	void zeroAll();
	bool internalCreate(in6_addr ip, uint16_t port, bool obtainDestination);

	int m_sock;
	uint16_t m_localPort;
	IPv6Address m_srcIP;
	uint16_t m_srcPort;
	IPv6Address m_dstIP;
	bool m_obtainDest;
};

} // end namespace

#endif // NUTCONFIG_SUPPORT_IPV6

#endif // NUT_UDPV6SOCKET_H

