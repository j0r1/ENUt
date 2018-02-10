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

#include "nutconfig.h"
#include "tcppacketsocket.h"
#include "packet.h"

#define TCPPACKETSOCKET_ERRSTR_COULDNTREADEXPECTEDAMOUNT	"Couln't read the expected amount of data"
#define TCPPACKETSOCKET_ERRSTR_READPACKETSIZETOOLARGE		"The received packet length exceeds the maximum allowed length"
#define TCPPACKETSOCKET_ERRSTR_NOPACKETSAVAILABLE		"There are currently no packets available"
#define TCPPACKETSOCKET_ERRSTR_PACKETTOOLARGE			"The specified length of the packet exceeds the maximum packet size"
#define TCPPACKETSOCKET_ERRSTR_COULDNTWRITEENTIRELENGTH		"Couldn't write all the bytes of the length field"
#define TCPPACKETSOCKET_ERRSTR_COULDNTWRITEENTIREIDENTIFIER	"Couldn't write all the bytes of the identifier"
#define TCPPACKETSOCKET_ERRSTR_COULDNTWRITEENTIREDATA		"Couldn't write all the data bytes"
#define TCPPACKETSOCKET_ERRSTR_READINVALIDIDENTIFIER		"An invalid identifier was read"

namespace nut
{

TCPPacketSocket::TCPPacketSocket(TCPSocket *pSock, bool deleteSocket, bool sixteenBit, uint32_t maxReceiveLength, size_t idLength, uint32_t id)
{
	m_pBaseSocket = pSock;
	m_deleteSocket = deleteSocket;

	m_maximumReceiveLength = maxReceiveLength;
	m_maximumSendLength = (sixteenBit)?0xffff:0xffffffff;
	m_lengthBytes = (sixteenBit)?sizeof(uint16_t):sizeof(uint32_t);
	m_sizeBufferPosition = 0;
	m_pBuffer = 0;
	m_bufferLength = 0;
	m_bufferPosition = 0;
	
	if (idLength > sizeof(uint32_t))
		m_idLength = sizeof(uint32_t);
	else if (idLength < 0)
		m_idLength = 0;
	else
		m_idLength = (int)idLength;
	m_id = id;
	m_idNBO = htonl(id);
	m_prefixLength = m_lengthBytes + m_idLength;

	m_lastWriteTime = time(0);
	m_lastReadTime = time(0);
}

TCPPacketSocket::~TCPPacketSocket()
{
	while (!m_packetQueue.empty())
	{
		Packet *pPack = *(m_packetQueue.begin());
		
		m_packetQueue.pop_front();
		delete pPack;
	}

	if (m_pBuffer)
		delete [] m_pBuffer;

	if (m_deleteSocket)
		delete m_pBaseSocket;
}

bool TCPPacketSocket::write(const void *pData, size_t length)
{
	uint32_t uLength = (uint32_t)length;

	if (uLength > m_maximumSendLength)
	{
		setErrorString(TCPPACKETSOCKET_ERRSTR_PACKETTOOLARGE);
		return false;
	}

	size_t varLength = m_lengthBytes;
	uint8_t sizeBuffer[sizeof(uint32_t)];
	int i,j;
	
	// write length field
	
	// This is big endian
	for (i = (m_lengthBytes-1)*8, j = 0 ; j < m_lengthBytes ; i -= 8, j++)
		sizeBuffer[j] = (uLength >> i)&0xff;
	
	if (!m_pBaseSocket->write(sizeBuffer, varLength))
	{
		setErrorString(m_pBaseSocket->getErrorString());
		return false;
	}

	if (varLength != m_lengthBytes)
	{
		setErrorString(TCPPACKETSOCKET_ERRSTR_COULDNTWRITEENTIRELENGTH);
		return false;
	}

	// write identifier if necessary
	
	if (m_idLength > 0)
	{
		varLength = (size_t)m_idLength;
		if (!m_pBaseSocket->write(&m_idNBO, varLength))
		{
			setErrorString(m_pBaseSocket->getErrorString());
			return false;
		}
		
		if (varLength != (size_t)m_idLength)
		{
			setErrorString(TCPPACKETSOCKET_ERRSTR_COULDNTWRITEENTIREIDENTIFIER);
			return false;
		}
	}
	
	// write actual data

	varLength = length;
	if (!m_pBaseSocket->write(pData, varLength))
	{
		setErrorString(m_pBaseSocket->getErrorString());
		return false;
	}

	if (varLength != length)
	{
		setErrorString(TCPPACKETSOCKET_ERRSTR_COULDNTWRITEENTIREDATA);
		return false;
	}

	m_lastWriteTime = time(0);

	return true;
}

bool TCPPacketSocket::read(Packet &packet)
{
	if (m_packetQueue.empty())
	{
		setErrorString(TCPPACKETSOCKET_ERRSTR_NOPACKETSAVAILABLE);
		return false;
	}

	Packet *pPacket;
	uint8_t *pData;
	size_t length;
	
	pPacket = *(m_packetQueue.begin());
	m_packetQueue.pop_front();

	pData = pPacket->extractData(length);
	delete pPacket;

	packet.setData(pData, length);

	m_lastReadTime = time(0);

	return true;
}

bool TCPPacketSocket::poll()
{
	size_t bytesToProcess = 0;
	bool done = false;

	if (!m_pBaseSocket->getAvailableDataLength(bytesToProcess))
	{
		setErrorString(m_pBaseSocket->getErrorString());
		return false;
	}

	while (!done)
	{
		if (m_pBuffer == 0) // No buffer is allocated, means we're still reading the length and (possibly) the identifier
		{
			size_t num = bytesToProcess;
			size_t sizeBufferSpace = m_prefixLength - m_sizeBufferPosition;
			
			if (bytesToProcess > sizeBufferSpace)
				num = sizeBufferSpace;
			
			size_t numToRead = num;
			if (!m_pBaseSocket->read((void *)(m_sizeBuffer + m_sizeBufferPosition), numToRead))
			{
				setErrorString(m_pBaseSocket->getErrorString());
				return false;
			}

			if (numToRead != num)
			{
				setErrorString(TCPPACKETSOCKET_ERRSTR_COULDNTREADEXPECTEDAMOUNT);
				return false;
			}
			
			m_sizeBufferPosition += num;
			bytesToProcess -= num;

			if ((int)m_sizeBufferPosition == m_prefixLength)
			{
				uint32_t uLength = 0;
				int i, j;

				for (i = (m_lengthBytes-1)*8, j = 0 ; j < m_lengthBytes ; i -= 8, j++)
					uLength |= (((uint32_t)m_sizeBuffer[j]) << i);

				if (uLength > m_maximumReceiveLength)
				{
					setErrorString(TCPPACKETSOCKET_ERRSTR_READPACKETSIZETOOLARGE);
					return false;
				}

				if (m_idLength > 0)
				{
					uint32_t id = 0;

					for (i = (m_idLength-1)*8 /* j simply continues */ ; j < m_prefixLength ; i -= 8, j++)
						id |= (((uint32_t)m_sizeBuffer[j]) << i);

					if (id != m_id)
					{
						setErrorString(TCPPACKETSOCKET_ERRSTR_READINVALIDIDENTIFIER);
						return false;
					}
				}
				
				m_bufferLength = uLength;
				m_bufferPosition = 0;
				m_pBuffer = new uint8_t[m_bufferLength];
				m_sizeBufferPosition = 0;
			}
			
			if (bytesToProcess == 0)
				done = true;
		}
		else // A buffer is allocated, keep filling it
		{
			size_t num = bytesToProcess;
			uint32_t uDiff = m_bufferLength - m_bufferPosition;
			
			if ((uint32_t)num > uDiff)
				num = (size_t)uDiff;

			size_t numToRead = num;
			if (!m_pBaseSocket->read((void *)(m_pBuffer + m_bufferPosition), numToRead))
			{
				setErrorString(m_pBaseSocket->getErrorString());
				return false;
			}

			if (numToRead != num)
			{
				setErrorString(TCPPACKETSOCKET_ERRSTR_COULDNTREADEXPECTEDAMOUNT);
				return false;
			}
			
			m_bufferPosition += num;
			bytesToProcess -= num;

			if (m_bufferPosition == m_bufferLength) // buffer full
			{
				Packet *pPacket = new Packet(m_pBuffer, (size_t)m_bufferLength);

				m_packetQueue.push_back(pPacket);
					
				m_pBuffer = 0;
				m_bufferPosition = 0;
				m_bufferLength = 0;
			}

			if (bytesToProcess == 0)
				done = true;
		}
	}
	return true;
}
	
} // end namespace


