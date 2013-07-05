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
#include "Link.h"
#include "EventLinkQueue.h"
#include "EventLinkDelay.h"
#include "ExceptionArgument.h"
#include "ExceptionUnsupported.h"

#define LINK_NODE_OUT(in) (in ^ 1)

namespace SimLib
{
	Link::Link(
		uint	id,
		SimHandler&	sim,
		uint	queue,
		__bitrate	bandwidth0,
		__bitrate	bandwidth1,
		__time		delay0,
		__time		delay1
		) : Object(sim), id(id)
	{
		this->bandwidth[0] = bandwidth0;
		this->bandwidth[1] = bandwidth1;
		this->delay[0] = delay0;
		this->delay[1] = delay1;

		this->nodes[0] = NULL;
		this->nodes[1] = NULL;

		this->nodeEntries[0] = 0;
		this->nodeEntries[1] = 0;

		// Low priority queues
		this->queues[0][0] = alloc LinkQueue(queue);
		this->queues[0][1] = alloc LinkQueue(queue);

		// High priority queues
		this->queues[1][0] = alloc LinkQueue(queue);
		this->queues[1][1] = alloc LinkQueue(queue);

		this->currentNode = 0;

		// Utilization meter
		this->meterTime[0] = 0;
		this->meterTime[1] = 0;
		this->meterUtil[0] = 0;
		this->meterUtil[1] = 0;

		// Statistics
		this->statUtil[0][0] = 0;
		this->statUtil[0][1] = 0;
		this->statUtil[1][0] = 0;
		this->statUtil[1][1] = 0;
	
		this->statQueue[0][0] = 0;
		this->statQueue[0][1] = 0;
		this->statQueue[1][0] = 0;
		this->statQueue[1][1] = 0;

		this->statQueueLast[0][0] = 0;
		this->statQueueLast[0][1] = 0;
		this->statQueueLast[1][0] = 0;
		this->statQueueLast[1][1] = 0;

		this->statPackets[0][0] = 0;
		this->statPackets[0][1] = 0;
		this->statPackets[1][0] = 0;
		this->statPackets[1][1] = 0;

		this->statBits[0][0] = 0;
		this->statBits[0][1] = 0;
		this->statBits[1][0] = 0;
		this->statBits[1][1] = 0;

		this->statDiscard[0][0] = 0;
		this->statDiscard[0][1] = 0;
		this->statDiscard[1][0] = 0;
		this->statDiscard[1][1] = 0;
	}

	Link::~Link()
	{
		delete this->queues[0][0];
		delete this->queues[0][1];
		delete this->queues[1][0];
		delete this->queues[1][1];
	}

	double Link::meterSmoothFactor = 0.9;

	uint Link::AddNode(Object* node, __byte entry)
	{
		if(NULL == node) throw ExceptionArgument(__FILE__, __LINE__, "Link::AddNode - node cannot be null.");
		if(2 < this->currentNode) throw ExceptionUnsupported(__FILE__, __LINE__, "Link::AddNode - link already has two nodes.");

		this->nodes[this->currentNode] = node;
		this->nodeEntries[this->currentNode] = entry;

		this->currentNode++;
	
		return this->currentNode-1;
	}

	Object::ERecvCode Link::Recv(Object* sender, __byte entry, Packet* packet)
	{
		if(NULL == sender) throw ExceptionArgument(__FILE__, __LINE__, "Link::Recv - sender cannot be null.");
		if(NULL == packet) throw ExceptionArgument(__FILE__, __LINE__, "Link::Recv - packet cannot be null.");
		if((0 != entry) && (1 != entry)) throw ExceptionArgument(__FILE__, __LINE__, "Link::Recv - entry can only be 0 or 1 (current value is %u).", entry);
		if(this->nodes[entry] != sender) throw ExceptionArgument(__FILE__, __LINE__, "Link::Recv - entry and sender mismatch (node at entry %u is %p, whereas sender is %p).", entry, this->nodes[entry], sender);

		__byte tos = packet->Tos();

		// Stat : packets
		this->statPackets[tos][entry]++;

		// If the queue is full discard the message
		if(this->queues[tos][entry]->IsFull())
		{
			// Verify that high priority packets are not discarded
			assert(tos == Packet::PACKET_TOS_LOW);

			// Stat : discarded packets
			this->statDiscard[tos][entry]++;

			packet->Delete();
			delete packet;
			return Object::RECV_FAIL;
		}

		// If all queues are empty, schedule a link event now to transmit the message
		if(this->queues[0][entry]->IsEmpty() && this->queues[1][entry]->IsEmpty())
		{
			__time delay = (packet->Size() + 1) / this->bandwidth[entry];

			EventLinkQueue* evt = alloc EventLinkQueue(this, entry, packet);
			this->sim.ScheduleEventAfter(
				delay,
				evt
				);

			// Stat : link utlization
			this->statUtil[tos][entry] += packet->Size() / this->bandwidth[entry];

			// Stat : data
			this->statBits[tos][entry] += packet->Size();

			// Utilization meter
			this->meterUtil[entry] = Link::meterSmoothFactor * packet->Size() / (this->sim.Time() + delay - this->meterTime[entry]) +
				(1 - Link::meterSmoothFactor) * this->meterUtil[entry];
			this->meterTime[entry] = this->sim.Time() + delay;
		}

		// Stat : queue
		this->statQueue[tos][entry] += this->queues[tos][entry]->Packets() * (this->sim.Time() - this->statQueueLast[tos][entry]);
		this->statQueueLast[tos][entry] = this->sim.Time();

		// Add the message to the queue
		this->queues[tos][entry]->Add(this->sim.Time(), packet);

		return RECV_SUCCESS;
	}

	void Link::Delay(__byte entry, Packet* packet)
	{
		if(NULL == packet) throw ExceptionArgument(__FILE__, __LINE__, "Link::Delay - packet cannot be null.");

		__byte tos = packet->Tos();

		if(this->queues[tos][entry]->First() != packet) throw ExceptionUnsupported(__FILE__, __LINE__, "Link::Delay - packet is not in link queue %u-%u.", tos, entry);

		// Stat : queue
		this->statQueue[tos][entry] += this->queues[tos][entry]->Packets() * (this->sim.Time() - this->statQueueLast[tos][entry]);
		this->statQueueLast[tos][entry] = this->sim.Time();

		// Remove packet from the queue
		this->queues[tos][entry]->Remove();

		// Delay the packet
		EventLinkDelay* evt = alloc EventLinkDelay(this, entry, packet);

		this->sim.ScheduleEventAfter(
			this->delay[entry],
			evt);

		// If the queue is not empty, process the next packet starting with the high priority queue
		for(__byte tos = Packet::PACKET_TOS_HIGH; tos <= Packet::PACKET_TOS_LOW; tos++)
		{
			if(!this->queues[tos][entry]->IsEmpty())
			{
				assert(this->queues[tos][entry]->First());

				Packet* packet = this->queues[tos][entry]->First();

				assert(packet->Tos() == tos);

				__time delay = (packet->Size() + 1) / this->bandwidth[entry];


				EventLinkQueue* evt = alloc EventLinkQueue(this, entry, packet);

				this->sim.ScheduleEventAfter(
					delay,
					evt
					);

				// Stat : link utlization
				this->statUtil[tos][entry] += packet->Size() / this->bandwidth[entry];

				// Stat : data
				this->statBits[tos][entry] += packet->Size();

				// Utilization meter
				this->meterUtil[entry] = Link::meterSmoothFactor * packet->Size() / (this->sim.Time() + delay - this->meterTime[entry]) +
				(1 - Link::meterSmoothFactor) * this->meterUtil[entry];
				this->meterTime[entry] = this->sim.Time() + delay;
	
				// If one packet is transmitted, stop the loop
				break;
			}
		}
	}

	void Link::Send(__byte entry, Packet* packet)
	{
		uint out = LINK_NODE_OUT(entry);

		// Call receive function of the destination node
		this->nodes[out]->Recv(this, this->nodeEntries[out], packet);
	}

	__bitrate Link::MeterUtil(__byte entry)
	{
		return this->meterUtil[entry];
	}

	void Link::Finalize()
	{
		this->statUtil[0][0] /= this->sim.Time();
		this->statUtil[0][1] /= this->sim.Time();
		this->statUtil[1][0] /= this->sim.Time();
		this->statUtil[1][1] /= this->sim.Time();

		this->statQueue[0][0] += this->queues[0][0]->Packets() * (this->sim.Time() - this->statQueueLast[0][0]);
		this->statQueue[0][1] += this->queues[0][1]->Packets() * (this->sim.Time() - this->statQueueLast[0][1]);
		this->statQueue[1][0] += this->queues[1][0]->Packets() * (this->sim.Time() - this->statQueueLast[1][0]);
		this->statQueue[1][1] += this->queues[1][1]->Packets() * (this->sim.Time() - this->statQueueLast[1][1]);

		this->statQueue[0][0] /= this->sim.Time();
		this->statQueue[0][1] /= this->sim.Time();
		this->statQueue[1][0] /= this->sim.Time();
		this->statQueue[1][1] /= this->sim.Time();
	}
}
