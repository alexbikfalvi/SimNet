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
#include "PacketTrfcFeedback.h"

namespace SimLib
{
	PacketTrfcFeedback::PacketTrfcFeedback(
		uint		flow,
		uint		src,
		uint		dst,
		__time		timeTx,
		__time		timeRx,
		__time		timeDelay,
		__bitrate	rateRecv,
		double		rateLoss,
		__uint64	ack,
		uint		ackSeq,
		__uint64	lost,
		uint		lostSeq
		) : PacketTrfc(32, PacketTrfc::FEEDBACK, flow, src, dst, Packet::PACKET_TOS_HIGH, timeTx, timeRx, timeDelay)
	{
		this->rateRecv = rateRecv;
		this->rateLoss = rateLoss;

		this->ack = ack;
		this->ackSeq = ackSeq;

		this->lost = lost;
		this->lostSeq = lostSeq;
	}

	Packet* PacketTrfcFeedback::Copy()
	{
		return alloc PacketTrfcFeedback(
			this->flow,
			this->src,
			this->dst,
			this->timeTx,
			this->timeRx,
			this->timeDelay,
			this->rateRecv,
			this->rateLoss,
			this->ack,
			this->ackSeq,
			this->lost,
			this->lostSeq
			);
	}
}