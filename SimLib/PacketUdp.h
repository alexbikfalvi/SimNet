/* 
 * Copyright (C) 2010-2012 Alex Bikfalvi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Packet.h"

#define PACKET_UDP_HEADER	64

namespace SimLib
{
	class PacketUdp : public Packet
	{
	protected:
		__uint16	src;
		__uint16	dst;
	
	public:
		PacketUdp(
			__uint16	src,
			__uint16	dst,
			Packet*	payload
			);
		virtual ~PacketUdp() { }

		inline __uint16				Src() { return this->src; }
		inline __uint16				Dst() { return this->dst; }

		virtual inline EPacketType	Type() { return PACKET_TYPE_UDP; }
		virtual Packet*				Copy();
	};
}
