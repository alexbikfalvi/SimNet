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

#include "PacketTrfc.h"

#define PACKET_INVALID_CONNECTION	0xFFFFFFFF

namespace SimLib
{
	class PacketTrfcMessage : public PacketTrfc
	{
	public:
		enum ETypeMessage
		{
			OPEN = 0,
			OPEN_ACK = 1,
			CLOSE = 2,
			CLOSE_ACK = 3,
			CLOSE_ACK_ACK = 4
		};

	private:
		// Message type
		ETypeMessage	typeMessage;

	public:
		PacketTrfcMessage(
			uint			flow,
			uint			src,
			__time			timeTx
			);
		PacketTrfcMessage(
			uint			flow,
			uint			src,
			uint			dst,
			ETypeMessage	type,
			__time			timeTx,
			__time			timeRx = -1
			);
		virtual ~PacketTrfcMessage() { }

		inline ETypeMessage	TypeMessage() { return this->typeMessage; }

		virtual Packet*		Copy();
	};
}
