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

#define PACKET_PIM_SM_HEADER	32

namespace SimLib
{
	class PacketPimSm : public Packet
	{
	public:
		enum EPacketPimSmType
		{
			HELLO = 0,
			REGISTER = 1,
			REGISTER_STOP = 2,
			JOIN_PRUNE = 3,
			BOOTSTRAP = 4,
			ASSERT = 5
		};

	protected:
		EPacketPimSmType	type;

	public:
		PacketPimSm(
			EPacketPimSmType	type,
			uint				size,
			Packet*				payload,
			ETypeOfService		tos
			);
		virtual ~PacketPimSm() { }

		inline EPacketPimSmType			PimSmType() { return this->type; }
		virtual inline EPacketType		Type() { return PACKET_TYPE_PIM_SM; }
	};
}
