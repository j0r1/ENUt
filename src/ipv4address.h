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
 * \file ipv4address.h
 */

#ifndef NUT_IPV4ADDRESS_H

#define NUT_IPV4ADDRESS_H

#include "nutconfig.h"
#include "networklayeraddress.h"
#include "nuttypes.h"

#if !(defined(WIN32) || defined(_WIN32_WCE))
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif // !(WIN32 || _WIN32_WCE)

namespace nut
{

/** An IPv4 address. */
class ENUT_IMPORTEXPORT IPv4Address : public NetworkLayerAddress
{
public:
	/** Create an instance based on the four bytes in \c ip. */
	IPv4Address(uint8_t ip[4]) : NetworkLayerAddress(IPv4)			{ m_ip = ((uint32_t)ip[3]) | (((uint32_t)ip[2]) << 8) | (((uint32_t)ip[1]) << 16) | (((uint32_t)ip[0]) << 24); }

	/** Create an instance based on the bytes \c ip0, \c ip1, \c ip2 and \c ip3. */
	IPv4Address(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3) : NetworkLayerAddress(IPv4)
										{ m_ip = ((uint32_t)ip3) | (((uint32_t)ip2) << 8) | (((uint32_t)ip1) << 16) | (((uint32_t)ip0) << 24); }

	/** Create an instance based on the 32-bit number \c ip, using the \c hostByteOrder 
	 *  flag to indicate its byte order. 
	 */
	IPv4Address(uint32_t ip = 0, bool hostByteOrder = true) : NetworkLayerAddress(IPv4)		
										{ if (hostByteOrder) m_ip = ip; else m_ip = ntohl(ip); }
	~IPv4Address()								{ }

	/** Returns the 32-bit IPv4 address in host byte order. */
	uint32_t getAddress() const						{ return m_ip; }

	/** Returns the 32-bit IPv4 address in network byte order. */
	uint32_t getAddressNBO() const						{ return htonl(m_ip); }

	/** Tries to set the address according to the string \c addressString.
	 *  Tries to set the address according to the string \c addressString.
	 *  Returns \c true if succesful, \c false otherwise.
	 */
	bool setAddress(const std::string &addressString);

	NetworkLayerAddress *createCopy() const					{ return new IPv4Address(m_ip); }
	bool isSameAddress(const NetworkLayerAddress &address) const;
	std::string getAddressString() const;
private:
	uint32_t m_ip;
};

inline bool IPv4Address::isSameAddress(const NetworkLayerAddress &address) const
{
	if (address.getProtocol() != IPv4)
		return false;
	
	const IPv4Address &addr = (const IPv4Address &)address;

	if (addr.m_ip != m_ip)
		return false;
	return true;
}

inline bool IPv4Address::setAddress(const std::string &addressString)
{
	uint32_t ip;

	ip = inet_addr(addressString.c_str());
	if (ip == INADDR_NONE)
		return false;
	m_ip = ntohl(ip);
	return true;
}
	
} // end namespace

#endif // NUT_IPV4ADDRESS_H

