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

#define PACKET_IGMP_HEADER	64

namespace SimLib
{
	class PacketIgmp : public Packet
	{
	public:
		enum EPacketIgmpType
		{
			PACKET_IGMP_MEMBERSHIP_QUERY = 0x11,
			PACKET_IGMP_MEMBERSHIP_REPORT = 0x16,
			PACKET_IGMP_LEAVE_GROUP = 0x17
		};

	protected:
		AddressIpv4		group;
		EPacketIgmpType	type;
		__time			maxResponse;

	public:
		PacketIgmp(
			AddressIpv4		group,
			EPacketIgmpType	type,
			__time			maxResponse = 0
			);
		virtual ~PacketIgmp() { }

		virtual inline EPacketType	Type() { return PACKET_TYPE_IGMP; }
		virtual Packet*				Copy();

		inline AddressIpv4			Group() { return this->group; }
		inline EPacketIgmpType		IgmpType() { return this->type; }
		inline __time				MaxResponse() { return this->maxResponse; }
	};
}