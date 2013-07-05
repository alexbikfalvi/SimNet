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

#include "PacketPimSm.h"
#include "AddressIpv4.h"

#define PACKET_PIM_SM_JOIN_PRUNE_HEADER	128

namespace SimLib
{
	class PacketPimSmJoinPrune : public PacketPimSm
	{
	public:
		enum EType
		{
			JOIN = 0,
			PRUNE = 1
		};
	protected:
		EType			type;
		AddressIpv4		address;

	public:
		PacketPimSmJoinPrune(
			EType			type,
			AddressIpv4		address
			);
		virtual ~PacketPimSmJoinPrune();

		virtual Packet*		Copy();

		inline EType		JoinPruneType() { return this->type; }
		inline AddressIpv4	Address() { return this->address; }
	};
}
