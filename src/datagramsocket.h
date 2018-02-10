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
 * \file datagramsocket.h
 */

#ifndef NUT_DATAGRAMSOCKET_H

#define NUT_DATAGRAMSOCKET_H

#include "nutconfig.h"
#include "nuttypes.h"
#include "socket.h"

namespace nut
{

class NetworkLayerAddress;

/** Interface for a datagram based protocol. */
class DatagramSocket : public Socket
{
protected:
	/** Create an unnamed instance. */
	DatagramSocket()										{ }

	/** Create an instance with the object name set to \c objName. */
	DatagramSocket(const std::string &objName) : Socket(objName)					{ }
public:
	~DatagramSocket()										{ }
	
	/** Join the multicast group specified in \c groupAddress. */
	virtual bool joinMulticastGroup(const NetworkLayerAddress &groupAddress) = 0;

	/** Leave the multicast group specified in \c groupAddress. */
	virtual bool leaveMulticastGroup(const NetworkLayerAddress &groupAddress) = 0;
	
	/** Sends data to a specified destination.
	 *  This function sends a certain amount of data to a specific address and port.
	 *  \param pData Data to be written.
	 *  \param length This parameter should contain the length of the data to be sent.
	 *                When the function returns, this value will be set to the actual
	 *                amount of data written.
	 *  \param destinationAddress Network address of the destination.
	 *  \param destinationPort Destination port number.
	 */
	virtual bool write(const void *pData, size_t &length, 
	                   const NetworkLayerAddress &destinationAddress, 
			   uint16_t destinationPort) = 0;
	
	/** This function can be used to check if data is available, and if so, how
	 *  many bytes are in the first packet.
	 *  This function can be used to check if data is available, and if so, how
	 *  many bytes are in the first packet.
	 *  \param length The length of the first available packet (can be zero!).
	 *  \param available Flag indicating if a packet is available.
	 *  \warning The meaning of the length field can differ from platform to platform.
	 *        With UDP sockets for example, on a Unix-like platform this call will
	 *        typically set the length to the length of the first UDP packet that
	 *        can be read. On a Windows platform the behavior is a bit different:
	 *        there the total length of all the queued messages is stored. This means
	 *        that you can use the length returned to allocate a buffer, but that
	 *        you MUST check the length set by the DatagramSocket::read call to
	 *        verify how many bytes have actually been stored in the buffer.
	 */
	virtual bool getAvailableDataLength(size_t &length, bool &available) = 0;

	/** Store the data of an available packet in the specified buffer.
	 *  Store the data of an available packet in the specified buffer.
	 *  \param pBuffer Buffer to store the data in.
	 *  \param bufferSize Initially, this should contain the size of the buffer. After
	 *                    completion, this contains the actual number of bytes stored.
	 */
	virtual bool read(void *pBuffer, size_t &bufferSize) = 0;

	/** Returns the source address of the last packet which was read. */
	virtual const NetworkLayerAddress *getLastSourceAddress() const = 0;

	/** Returns the source port of the last packet which was read. */
	virtual uint16_t getLastSourcePort() const = 0;

	/** Returns the destination address which was stored in the last read packet.
	 *  Returns the destination address which was stored in the last read packet.
	 *  If not supported on the current platform or if this feature was not requested,
	 *  the function will return NULL. This function can be useful when the packet
	 *  was sent to a multicast or broadcast address.
	 */
	virtual const NetworkLayerAddress *getLastDestinationAddress() const = 0;
};
	
} // end namespace

#endif // NUT_DATAGRAMSOCKET_H
