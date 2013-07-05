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
#include "PacketIgmp.h"

namespace SimLib
{
	PacketIgmp::PacketIgmp(
		AddressIpv4		group,
		EPacketIgmpType	type,
		__time			maxResponse
		) : Packet(PACKET_IGMP_HEADER, NULL, PACKET_TOS_HIGH),
		group(group), type(type), maxResponse(maxResponse)
	{
	}

	Packet* PacketIgmp::Copy()
	{
		Packet* packet = alloc PacketIgmp(
			this->group,
			this->type,
			this->maxResponse);

		return packet;
	}
}