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

/**
 * \file multicasttunnelserver.h
 */

#ifndef MULTICASTTUNNELSERVER_H

#define MULTICASTTUNNELSERVER_H

#define MCASTTUNNEL_CHANNEL_CONTROL			0
#define MCASTTUNNEL_CHANNEL_DATA			1

#define MCASTTUNNEL_COMMAND_PORT			1
#define MCASTTUNNEL_COMMAND_ACK				2
#define MCASTTUNNEL_COMMAND_ACKERR			3
#define MCASTTUNNEL_COMMAND_START			4
#define MCASTTUNNEL_COMMAND_JOIN			5
#define MCASTTUNNEL_COMMAND_LEAVE			6
	
#endif // MULTICASTTUNNELSERVER_H
