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

#ifdef NUTCONFIG_SUPPORT_IPV6

#include "ipv6address.h"

namespace nut
{

std::string IPv6Address::getAddressString() const
{
	char str[48];
	uint16_t ip16[8];
	int i,j;

	for (i = 0,j = 0 ; j < 8 ; j++,i += 2)
	{
		ip16[j] = (((uint16_t)m_ip.s6_addr[i])<<8);
		ip16[j] |= ((uint16_t)m_ip.s6_addr[i+1]);
	}

        snprintf(str,48,"%04X:%04X:%04X:%04X:%04X:%04X:%04X:%04X",(int)ip16[0],(int)ip16[1],(int)ip16[2],(int)ip16[3],
	                                                          (int)ip16[4],(int)ip16[5],(int)ip16[6],(int)ip16[7]);
	return std::string(str);
}

} // end namespace

#endif // NUTCONFIG_SUPPORT_IPV6

