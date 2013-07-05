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

namespace SimLib
{
	enum EPacketType
	{
		PACKET_TYPE_UNKNOWN = 0,
		PACKET_TYPE_IP = 1,
		PACKET_TYPE_UDP = 2,
		PACKET_TYPE_IGMP = 3,
		PACKET_TYPE_PIM_SM = 4,
		PACKET_TYPE_STREAM = 5,
		PACKET_TYPE_MESSAGE = 6,
		PACKET_TYPE_CONNECTION = 7,
		PACKET_TYPE_DUMMY = 8
	};

	class Packet
	{
	public:
		enum ETypeOfService
		{
			PACKET_TOS_HIGH = 0,
			PACKET_TOS_LOW = 1
		};

	protected:
		uint			size;	
		Packet*			payload;
		ETypeOfService	tos;

	public:
		Packet(
			uint			size,
			Packet*			payload,
			ETypeOfService	tos = PACKET_TOS_LOW
			);
		virtual ~Packet() { }

		inline uint				Size() { return this->size; }
		inline Packet*			Payload() { return this->payload; }
		inline ETypeOfService	Tos() { return this->tos; }
	
		virtual EPacketType		Type() = 0;
		virtual Packet*			Copy() = 0;

		void					Delete();

		virtual inline char*	ToString() { return NULL; }
	};
}
