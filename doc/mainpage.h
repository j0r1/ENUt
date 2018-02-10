/**
 * \mainpage ENUt
 *
 * \author Hasselt University - Expertise Centre for Digital Media
 *
 * \section intro Introduction
 *
 * 	ENUt stands for 'EDM Network Utilities', a library meant to make network
 * 	programming a bit easier.
 *
 * \section license License
 *
 * 	The license which applies to this library is the LGPL. You can find the
 * 	full version in the file \c LICENSE.LGPL which is included in the library
 * 	archive. The short version is the following:
 *
 *	\code
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  
 * USA.
 *	\endcode
 *
 * \section using Using the library
 *
 * 	For the most part, the library contains simple C++ wrappers for BSD-style
 * 	socket functions. For example, there's a nut::UDPv4Socket which can be
 * 	used for sending and receiving UDP packets over the IPv4 protocol. The
 * 	'select' function is replaced by a nut::SocketWaiter class. Just take a
 * 	look at the available classes, most of them will be clear immediately.
 *
 * 	The different socket classes use some additional internal variables, which
 * 	are not protected by a mutex. This means that the library is not thread
 * 	safe; if a socket class is used by multiple threads, a mutex should be 
 * 	used to lock the socket.
 *
 * \section contact Contact
 *
 * 	The library homepage can be found at the following location:
 * 	http://research.edm.uhasselt.be/enut/
 * 		
 * 	You can reach the ENUt developers by sending an e-mail to
 *	\c enutinfo \c [\c at] \c edm \c [\c dot] \c uhasselt \c [\c dot] be 
 * 
 */

