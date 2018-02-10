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
 * \file tcppacketsocket.h
 */

#ifndef NUT_TCPPACKETSOCKET_H

#define NUT_TCPPACKETSOCKET_H

#include "nutconfig.h"
#include "tcpsocket.h"
#include <time.h>
#include <list>

namespace nut
{

class TCPSocket;
class Packet;
	
/** A class which allows you to send and receive packets over a TCP connection. */
class ENUT_IMPORTEXPORT TCPPacketSocket : public Socket
{
public:
	/** Create a socket which can be used to send packets over a TCP connection.
	 *  Create a socket which can be used to send packets over a TCP connection. In
	 *  16-bit mode, the framing method used is compatible with the one described
	 *  in RFC 4571.
	 *  \param pSock The TCP socket which will be used to send and receive packets.
	 *               The socket should be in a connected state before attempting
	 *               to read or write data packets.
	 *  \param deleteSocket If true, the TCP socket will be deleted when the TCPPacketSocket
	 *                      instance is destroyed.
	 *  \param sixteenBit If true, a 16 bit length field will be used in the
	 *                    framing method. Otherwise, a 32 bit length field is used.
	 *                    The length field will be encoded in netword byte order
	 *                    (big endian).
	 *  \param maxReceiveLength Packets containing a length field which exceeds this
	 *                          value will generate an error.
	 *  \param idLength If not zero, this many bytes of the \c id parameter will also
	 *                  be stored in each packet. Packet reading will fail if the bytes
	 *                  are not present in a packet.
	 *  \param id An optional identifier which can be stored in each packet.
	 */
	TCPPacketSocket(TCPSocket *pSock, bool deleteSocket, bool sixteenBit = false, uint32_t maxReceiveLength = 0xffffffff, size_t idLength = 0, uint32_t id = 0);
	
	~TCPPacketSocket();

	/** This function checks if data is available on the underlying TCP socket and
	 *  stores all the available packets in an internal queue.
	 */
	bool poll();

	/** Returns the amount of data that is available on the underlying TCP socket. */
	bool getAvailableDataLength(size_t &length)						{ if (!m_pBaseSocket->getAvailableDataLength(length)) { setErrorString(m_pBaseSocket->getErrorString()); return false; } return true; }

	/** This function checks if packets are stored in the internal packet queue.
	 *  This function checks if packets are stored in the internal packet queue. Incoming 
	 *  packets are stored in this queue by calling the TCPPacketSocket::poll function.
	 *  Packets can be extracted from this queue by calling the TCPPacketSocket::read function.
	 */
	bool isPacketAvailable()								{ return !m_packetQueue.empty(); }
	
	/** Extract the first packet which is stored in the internal queue and store the data
	 *  in \c packet.
	 *  Extract the first packet which is stored in the internal queue and store the data
	 *  in \c packet. Incoming packets are stored in this queue by calling the 
	 *  TCPPacketSocket::poll function.
	 */
	bool read(Packet &packet);
	
	/** Transmit a packet of length \c length, containing the data specified in \c pData. */
	bool write(const void *pData, size_t length);

	time_t getLastReadTime() const								{ return m_lastReadTime; }
	time_t getLastWriteTime() const								{ return m_lastWriteTime; }
	TCPSocket *getBaseSocket()								{ return m_pBaseSocket; }
protected:
#if (defined(WIN32) || defined(_WIN32_WCE))
	SOCKET getSocketDescriptor()								{ return m_pBaseSocket->getSocketDescriptor(); }
#else
	int getSocketDescriptor()								{ return m_pBaseSocket->getSocketDescriptor(); }
#endif // WIN32 || _WIN32_WCE
private:
	// make sure we can't copy the socket
	TCPPacketSocket(const TCPPacketSocket &s) : Socket(s)					{ }
	TCPPacketSocket &operator=(const TCPPacketSocket &s)					{ return *this; }

	TCPSocket *m_pBaseSocket;
	uint32_t m_maximumSendLength;
	uint32_t m_maximumReceiveLength;
	uint32_t m_id, m_idNBO;
	int m_idLength;
	int m_lengthBytes;
	int m_prefixLength;
	bool m_deleteSocket;

	std::list<Packet *> m_packetQueue;

	uint8_t m_sizeBuffer[sizeof(uint32_t)*2]; // one for the length field, the other one for an optional 32-bit identifier
	size_t m_sizeBufferPosition;
	
	uint8_t *m_pBuffer;
	uint32_t m_bufferLength;
	uint32_t m_bufferPosition;

	time_t m_lastReadTime;
	time_t m_lastWriteTime;
};

} // end namespace

#endif // NUT_TCPPACKETSOCKET_H

