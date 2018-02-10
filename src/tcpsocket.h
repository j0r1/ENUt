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
 * \file tcpsocket.h
 */

#ifndef NUT_TCPSOCKET_H

#define NUT_TCPSOCKET_H

#include "nutconfig.h"
#include "nuttypes.h"
#include "socket.h"
#include "networklayeraddress.h"

namespace nut
{

/** General TCP socket interface. */
class ENUT_IMPORTEXPORT TCPSocket : public Socket
{
protected:
	TCPSocket(NetworkLayerProtocol proto)						{ m_protocol = proto; }
	TCPSocket(const std::string &objName, NetworkLayerProtocol proto) : Socket(objName)
											{ m_protocol = proto; }
public:
	virtual ~TCPSocket()								{ }

	/** Returns the underlying network layer protocol which is used. */
	NetworkLayerProtocol getProtocol() const					{ return m_protocol; }

	/** Creates a TCP socket.
	 *  Creates a TCP socket.
	 *  \param portNumber If not zero, the socket will be bound to this port number.
	 */
	virtual bool create(uint16_t portNumber = 0) = 0;

	/** Creates a TCP socket.
	 *  Creates a TCP socket.
	 *  \param bindAddress The socket will be bound to this address.
	 *  \param portNumber If not zero, the socket will be bound to this port number.
	 */
	virtual bool create(const NetworkLayerAddress &bindAddress, uint16_t portNumber = 0) = 0;

	/** Destroys the socket. */
	virtual bool destroy() = 0;

	/** Can be used to set the socket to either non-blocking or blocking mode. */
	virtual bool setNonBlocking(bool f = true) = 0;
	
	/** Returns the port number to which the socket is bound. */
	virtual uint16_t getLocalPortNumber() = 0;

	/** Try to establish a TCP connection with a specified end-point.
	 *  Try to establish a TCP connection with a specified end-point.
	 *  \param hostAddress IP address of the end-point.
	 *  \param portNumber Port number of the end-point.
	 */
	virtual bool connect(const NetworkLayerAddress &hostAddress, uint16_t portNumber) = 0;

	/** Puts the socket in listen mode.
	 *  Puts the socket in listen mode, so that incoming connections can be accepted.
	 *  \param backlog The maximum length of the queue of pending connections.
	 */
	virtual bool listen(int backlog) = 0;

	/** Accepts an incoming connection request.
	 *  Accepts an incoming connection request.
	 *  \param pNewSock Will contain a new socket which can be used for communication.
	 */
	virtual bool accept(TCPSocket **pNewSock) = 0;

	/** Returns true if a TCP connection has been established. */
	virtual bool isConnected() = 0;
	
	/** Returns the address of the other end of the TCP connection. 
	 *  Returns the address of the other end of the TCP connection.	 
	 */
	virtual const NetworkLayerAddress *getDestinationAddress() const = 0;

	/** Returns the port number of the other end of the TCP connection. */
	virtual uint16_t getDestinationPort() const = 0;
	
	/** Writes data over the TCP connection.
	 *  Writes data over the TCP connection.
	 *  \param pData Pointer to the data which should be transmitted.
	 *  \param length Initially, this should contain the number of bytes
	 *                which should be transmitted. When the function returns,
	 *                the variable will contain the number of bytes which
	 *                were actually transmitted.
	 */
	virtual bool write(const void *pData, size_t &length) = 0;

	/** Stores the number of bytes which can be read from the connection in \c length. */
	virtual bool getAvailableDataLength(size_t &length) = 0;

	/** Reads data from the TCP connection.
	 *  Reads data from the TCP connection.
	 *  \param pBuffer Pointer to the buffer where the data can be stored.
	 *  \param bufferSize Initially, this should contain the maximum amount of
	 *                    bytes which may be stored in \c pBuffer. When the function
	 *                    returns, the variable contains the actual number of bytes
	 *                    read.
	 */
	virtual bool read(void *pBuffer, size_t &bufferSize) = 0;
private:
	NetworkLayerProtocol m_protocol;

	friend class TCPPacketSocket;
};

}

#endif // NUT_TCPSOCKET_H

