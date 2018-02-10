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
 * \file networklayeraddress.h
 */

#ifndef NUT_NETWORKLAYERADDRESS_H

#define NUT_NETWORKLAYERADDRESS_H

#include "nutconfig.h"
#include <string>

/** Namespace. */
namespace nut
{

/** List of supported network layer protocols. */
enum NetworkLayerProtocol 
{ 	
	/** Internet Protocol (IP) version 4. */
	IPv4,	

	/** Internet Protocol (IP) version 6. */
	IPv6	
};
	
/** Base class for network layer addresses. */
class ENUT_IMPORTEXPORT NetworkLayerAddress
{
protected:
	NetworkLayerAddress(NetworkLayerProtocol p)					{ m_protocol = p; }
public:
	virtual ~NetworkLayerAddress()							{ }

	/** Returns the protocol used by this instance. */
	NetworkLayerProtocol getProtocol() const					{ return m_protocol; }	
	
	/** Creates a copy of the current address. */
	virtual NetworkLayerAddress *createCopy() const = 0;

	/** Check if this address is the same as the one stored in \c address. */
	virtual bool isSameAddress(const NetworkLayerAddress &address) const = 0;

	/** Converts the address contained in this instance to a readable string. */
	virtual std::string getAddressString() const = 0;
private:
	NetworkLayerProtocol m_protocol;
};
	
}

#endif // NUT_NETWORKLAYERADDRESS_H

