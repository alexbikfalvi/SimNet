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

namespace SimLib
{
	class PacketTrfc : public Packet
	{
	public:
		enum ETypeConnection
		{
			DATA = 0,
			FEEDBACK = 1,
			MESSAGE = 2
		};

	protected:
		// Packet type
		ETypeConnection	typeConnection;

		// Connections
		uint			src;
		uint			dst;

		// Stream
		uint			flow;

		// Flow
		__time			timeTx;
		__time			timeRx;
		__time			timeDelay;

	public:
		PacketTrfc(
			uint			size,
			ETypeConnection	typeConnection,
			uint			flow,
			uint			src,
			uint			dst,
			ETypeOfService	tos,
			__time			timeTx,
			__time			timeRx = -1,
			__time			timeDelay = 0
			);
		virtual ~PacketTrfc() { }

		virtual EPacketType		Type() { return PACKET_TYPE_CONNECTION; }
		ETypeConnection			TypeConnection() { return this->typeConnection; }

		inline uint				Src() { return this->src; }
		inline uint				Dst() { return this->dst; }
		inline uint				Flow() { return this->flow; }

		inline __time			TimeTx() { return this->timeTx; }
		inline __time			TimeRx() { return this->timeRx; }
		inline __time			TimeDelay() { return this->timeDelay; }
	};
}
