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
#include "AddressIpv4.h"

#define PACKET_IP_HEADER	160

namespace SimLib
{
	class PacketIpv4 : public Packet
	{
	protected:
		AddressIpv4	src;
		AddressIpv4	dst;
		__byte		ttl;

	public:
		PacketIpv4(
			AddressIpv4	src,
			AddressIpv4	dst,
			__byte		ttl,
			Packet*		payload
			);
		virtual ~PacketIpv4() { }

		inline AddressIpv4			Src() { return this->src; }
		inline AddressIpv4			Dst() { return this->dst; }
		inline __byte				Ttl() { return this->ttl; }
	
		inline void					DecTtl() { this->ttl--; }

		virtual inline EPacketType	Type() { return PACKET_TYPE_IP; }
		virtual Packet*				Copy();
	};
}
