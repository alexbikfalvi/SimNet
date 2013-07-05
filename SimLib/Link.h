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

#include "Object.h"
#include "LinkQueue.h"

namespace SimLib
{
	class Link : public Object
	{
	protected:
		uint		id;
		Object*			nodes[2];
		uint		nodeEntries[2];
		LinkQueue*		queues[2][2];
		uint		currentNode;

		__bitrate		bandwidth[2];
		__time			delay[2];
	
		double			statUtil[2][2];
		__uint64		statPackets[2][2];
		__uint64		statDiscard[2][2];
		__uint64		statBits[2][2];
		double			statQueue[2][2];
		__time			statQueueLast[2][2];

		__time			meterTime[2];
		__bitrate		meterUtil[2];

		static double	meterSmoothFactor;

	public:
		Link(
			uint	id,
			SimHandler&	sim,
			uint	queue,
			__bitrate	bandwidth0,
			__bitrate	bandwidth1,
			__time		delay0,
			__time		delay1
			);
		virtual ~Link();

		inline uint			Id() { return this->id; }
		inline __bitrate		Bandwidth(__byte entry) { return this->bandwidth[entry]; }

		uint				AddNode(Object* node, __byte entry);
		inline Object*			Node(uint index) { assert(index < 2); return this->nodes[index]; }
		inline uint			NodeEntry(uint index) { assert(index < 2); return this->nodeEntries[index]; }

		virtual ERecvCode		Recv(Object* sender, __byte entry, Packet* packet);
		void					Delay(__byte entry, Packet* packet);
		void					Send(__byte entry, Packet* packet);

		virtual void			Finalize();

		inline LinkQueue*		Queue(__byte tos, __byte entry) { return this->queues[tos][entry]; }

		__bitrate				MeterUtil(__byte entry);
		uint				MeterQueue(__byte tos, __byte entry) { return this->queues[tos][entry]->Packets(); }

		inline double			StatUtilization(__byte tos, __byte entry) { return this->statUtil[tos][entry]; }
		inline __uint64			StatPackets(__byte tos, __byte entry) { return this->statPackets[tos][entry]; }
		inline __uint64			StatBits(__byte tos, __byte entry) { return this->statBits[tos][entry]; }
		inline __uint64			StatDiscard(__byte tos, __byte entry) { return this->statDiscard[tos][entry]; }
		inline double			StatQueue(__byte tos, __byte entry) { return this->statQueue[tos][entry]; }
	};
}
