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

#include "nutconfig.h"
#include "ipv4address.h"
#include <stdio.h>

namespace nut
{
	
std::string IPv4Address::getAddressString() const
{
	char str[16];

#if (defined(WIN32) || defined(_WIN32_WCE))
	_snprintf(str, 16, "%d.%d.%d.%d", (int)((m_ip >> 24)&0xff), (int)((m_ip >> 16)&0xff), (int)((m_ip >> 8)&0xff), (int)(m_ip&0xff));
#else
	snprintf(str, 16, "%d.%d.%d.%d", (int)((m_ip >> 24)&0xff), (int)((m_ip >> 16)&0xff), (int)((m_ip >> 8)&0xff), (int)(m_ip&0xff));
#endif // WIN32 || _WIN32_WCE
	return std::string(str);
}

} // end namespace

