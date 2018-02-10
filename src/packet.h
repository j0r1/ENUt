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
 * \file packet.h
 */

#ifndef NUT_PACKET_H

#define NUT_PACKET_H

#include "nutconfig.h"
#include "nuttypes.h"

namespace nut
{

/** Container for a pointer to data and a length field. */
class ENUT_IMPORTEXPORT Packet
{
public:
	/** Construct an empty packet. */
	Packet()									{ m_pData = 0; m_length = 0; }

	/** Construct a packet containig specified data.
	 *  Construct a packet containig specified data. When this instance
	 *  is destroyed, the data will be deleted as well. To prevent the
	 *  data from getting lost, the Packet::extractData member function
	 *  can be used.
	 *  \param pData Pointer to the data which should be stored.
	 *  \param length Length of the data.
	 */
	Packet(uint8_t *pData, size_t length)						{ m_pData = pData; m_length = length; }

	~Packet()									{ if (m_pData) delete [] m_pData; }

	/** Store the specified data.
	 *  Store the specified data. When this instance
	 *  is destroyed, the data will be deleted as well. To prevent the
	 *  data from getting lost, the Packet::extractData member function
	 *  can be used.
	 *  \param pData Pointer to the data which should be stored.
	 *  \param length Length of the data.
	 */
	void setData(uint8_t *pData, size_t length)					{ if (m_pData) delete [] m_pData; m_pData = pData; m_length = length; }

	/** Returns a pointer to the data. */
	const uint8_t *getData() const							{ return m_pData; }

	/** Returns the length of the data. */
	size_t getLength() const							{ return m_length; }

	/** Returns a pointer to the data and sets the stored pointer to null.
	 *  Returns a pointer to the data and sets the stored pointer to null.
	 *  After this call, you have to delete the returned data somewhere
	 *  yourself.
	 *  \param length The length of the data which was returned.
	 */
	uint8_t *extractData(size_t &length)						{ uint8_t *pData = m_pData; length = m_length; m_pData = 0; m_length = 0; return pData; }
private:
	// Make sure we don't accidentally copy such objects
	Packet(const Packet &p)								{ }
	Packet &operator=(const Packet &p)						{ return *this; }

	uint8_t *m_pData;
	size_t m_length;
};
	
} // end namespace

#endif // NUT_PACKET_H

