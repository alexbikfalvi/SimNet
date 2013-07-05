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

namespace SimLib
{
	class PacketTrfcData : public PacketTrfc
	{
	private:
		// Flow
		uint	sequence;
		__time	rtt;

	public:
		PacketTrfcData(
			uint		flow,
			uint		src,
			uint		dst,
			Packet*		payload,
			uint		sequence,
			__time		timeTx,
			__time		rtt
			);
		virtual ~PacketTrfcData() { }

		inline uint		Sequence() { return this->sequence; }
		inline __time	Rtt() { return this->rtt; }
	
		virtual Packet*	Copy();
	};
}
