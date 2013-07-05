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

#include "Headers.h"
#include "PacketIpv4.h"

namespace SimLib
{
	PacketIpv4::PacketIpv4(
		AddressIpv4	src,
		AddressIpv4	dst,
		__byte		ttl,
		Packet*		payload
		) : Packet(PACKET_IP_HEADER + (payload?payload->Size():0), payload, payload?payload->Tos():PACKET_TOS_LOW),
		src(src),
		dst(dst),
		ttl(ttl)
	{
	}

	Packet* PacketIpv4::Copy()
	{
		Packet* packet = alloc PacketIpv4(
			this->src,
			this->dst,
			this->ttl,
			this->payload->Copy());

		return packet;
	}
}