/*
    
  This file is a part of ENUt, a library containing network
  programming utilities.
  
  Copyright (C) 2006-2008  Hasselt University - Expertise Centre for
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
 * \file udpsocket.h
 */

#ifndef NUT_UDPSOCKET_H

#define NUT_UDPSOCKET_H

#include "nutconfig.h"
#include "nuttypes.h"
#include "datagramsocket.h"
#include "networklayeraddress.h"

namespace nut
{

/** General UDP socket interface. */
class UDPSocket : public DatagramSocket
{
protected:
	UDPSocket(NetworkLayerProtocol proto)						{ m_protocol = proto; }
	UDPSocket(const std::string &objName, NetworkLayerProtocol proto) : DatagramSocket(objName)
											{ m_protocol = proto; }
public:
	virtual ~UDPSocket()								{ }

	/** Returns the underlying network layer protocol which this socket uses. */
	NetworkLayerProtocol getProtocol() const					{ return m_protocol; }

	/** Create an UDP socket.
	 *  Create an UDP socket.
	 *  \param portNumber If not zero, the socket will be bound to this port number.
	 *  \param obtainDestination If true, the network layer destination address which 
	 *                           was stored in the UDP packet will be retrieved. This
	 *                           can come in handy is the packet was sent to a multicast
	 *                           address and your application needs to retrieve this
	 *                           multicast address somehow. Note that this is not
	 *                           supported on all platforms.
	 */
	virtual bool create(uint16_t portNumber = 0, bool obtainDestination = false) = 0;

	/** Create an UDP socket.
	 *  Create an UDP socket.
	 *  \param bindAddress The network layer address to which this socket will be bound.
	 *  \param portNumber If not zero, the socket will be bound to this port number.
	 *  \param obtainDestination If true, the network layer destination address which 
	 *                           was stored in the UDP packet will be retrieved. This
	 *                           can come in handy is the packet was sent to a multicast
	 *                           address and your application needs to retrieve this
	 *                           multicast address somehow. Note that this is not
	 *                           supported on all platforms.
	 */
	virtual bool create(NetworkLayerAddress &bindAddress, uint16_t portNumber = 0, bool obtainDestination = false) = 0;
	
	/** Destroys the socket. */
	virtual bool destroy() = 0;

	/** Can be used to set the socket in either non-blocking or blocking mode. */
	virtual bool setNonBlocking(bool f = true) = 0;

	/** Returns the port number to which the socket is bound. */
	virtual uint16_t getLocalPortNumber() const = 0;

	/** Sets the Time To Live (TTL) field for outgoing multicast packets. */
	virtual bool setMulticastTTL(uint8_t ttl) = 0;
private:
	NetworkLayerProtocol m_protocol;
};
	
}

#endif // NUT_UDPSOCKET_H
