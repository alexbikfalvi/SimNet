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
#include "PacketPimSmJoinPrune.h"

namespace SimLib
{
	PacketPimSmJoinPrune::PacketPimSmJoinPrune(
		EType		type,
		AddressIpv4	address
		) : PacketPimSm(
		PacketPimSm::JOIN_PRUNE,
		PACKET_PIM_SM_JOIN_PRUNE_HEADER,
		NULL,
		Packet::PACKET_TOS_HIGH)
	{
		this->type = type;
		this->address = address;
	}

	PacketPimSmJoinPrune::~PacketPimSmJoinPrune()
	{
	}

	Packet* PacketPimSmJoinPrune::Copy()
	{
		Packet* packet = alloc PacketPimSmJoinPrune(
			this->type,
			this->address);

		return packet;
	}
}