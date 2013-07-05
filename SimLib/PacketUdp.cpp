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
#include "PacketUdp.h"

namespace SimLib
{
	PacketUdp::PacketUdp(
		__uint16	src,
		__uint16	dst,
		Packet*	payload
		) : Packet(PACKET_UDP_HEADER + (payload?payload->Size():0), payload, payload?payload->Tos():PACKET_TOS_LOW)
	{
		this->src = src;
		this->dst = dst;
	}

	Packet* PacketUdp::Copy()
	{
		Packet* packet = alloc PacketUdp(
			this->src,
			this->dst,
			this->payload->Copy());

		return packet;
	}
}
