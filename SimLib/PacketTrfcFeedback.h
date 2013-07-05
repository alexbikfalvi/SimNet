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
	class PacketTrfcFeedback : public PacketTrfc
	{
	private:
		__bitrate	rateRecv;
		double		rateLoss;
	
		// Acknowledged packets
		__uint64	ack;
		uint		ackSeq;

		// Lost packets
		__uint64	lost;
		uint		lostSeq;

	public:
		PacketTrfcFeedback(
			uint		flow,
			uint		src,
			uint		dst,
			__time		timeTx,
			__time		timeRx,
			__time		timeDelay,
			__bitrate	rateRecv,
			double		rateLoss,
			__uint64	ack = 0,
			uint		ackSeq = 0,
			__uint64	lost = 0,
			uint		lostSeq = 0
			);
		virtual ~PacketTrfcFeedback() { }

		inline __bitrate	RateRecv() { return this->rateRecv; }
		inline double		RateLoss() { return this->rateLoss; }

		inline __uint64		Ack() { return this->ack; }
		inline __uint64		Lost() { return this->lost; }

		inline uint			AckSeq() { return this->ackSeq; }
		inline uint			LostSeq() { return this->lostSeq; }

		virtual Packet*		Copy();
	};
}
