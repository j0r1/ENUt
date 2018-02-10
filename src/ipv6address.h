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
 * \file ipv6address.h
 */

#ifndef NUT_IPV6ADDRESS_H

#define NUT_IPV6ADDRESS_H

#include "nutconfig.h"

#ifdef NUTCONFIG_SUPPORT_IPV6

#include "networklayeraddress.h"
#if ! (defined(WIN32) || defined(_WIN32_WCE))
	#include <netinet/in.h>
#endif

namespace nut
{

/** An IPv6 address. */
class ENUT_IMPORTEXPORT IPv6Address : public NetworkLayerAddress
{
public:
	IPv6Address() : NetworkLayerAddress(IPv6)						{ for (int i = 0 ; i < 16 ; i++) m_ip.s6_addr[i] = 0; }

	/** Creates an instance based on \c ip. */
	IPv6Address(in6_addr ip) : NetworkLayerAddress(IPv6)					{ ip = m_ip; }

	/** Creates an instance based on the sixteen bytes in \c ip. */
	IPv6Address(const uint8_t ip[16]) : NetworkLayerAddress(IPv6)				{ for (int i = 0 ; i < 16 ; i++) m_ip.s6_addr[i] = ip[i]; }
	~IPv6Address()										{ }	

	/** Returns the address in the form of an \c in6_addr instance. */
	in6_addr getAddress() const								{ return m_ip; }

	NetworkLayerAddress *createCopy() const							{ return new IPv6Address(m_ip); }
	bool isSameAddress(const NetworkLayerAddress &address) const;
	std::string getAddressString() const;
private:
	in6_addr m_ip;
};

inline bool IPv6Address::isSameAddress(const NetworkLayerAddress &address) const
{
	if (address.getProtocol() != IPv6)
		return false;
	
	const IPv6Address &addr = (const IPv6Address &)address;

	for (int i = 0 ; i < 16 ; i++)
	{
		if (m_ip.s6_addr[i] != addr.m_ip.s6_addr[i])
			return false;
	}
	return true;
}

} // end namespace

#endif // NUTCONFIG_SUPPORT_IPV6

#endif // NUT_IPV6ADDRESS_H

