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
#include "LinkQueue.h"
#include "ExceptionArgument.h"
#include "ExceptionUnsupported.h"

namespace SimLib
{
	LinkQueue::LinkQueue(
		uint	size
		)
	{
		this->queue.resize(size, NULL);
		this->time.resize(size, 0);
		this->ptr = 0;
		this->count = 0;
		this->usage = 0;
	}

	LinkQueue::~LinkQueue()
	{
		// Delete packets stored in the queue (start from 1, because event 0 has an associated queue event and will be deleted with the event)
		for(uint index = 1; index < this->count; index++)
		{
			uint idx = (this->ptr + index) % this->queue.size();

			this->queue[idx]->Delete();
			delete this->queue[idx];
		}
	}

	void LinkQueue::Add(__time time, Packet* packet)
	{
		if(NULL == packet) throw ExceptionArgument(__FILE__, __LINE__, "LinkQueue::Add - packet cannot be null.");
		if(this->count >= this->queue.size()) throw ExceptionUnsupported(__FILE__, __LINE__, "LinkQueue::Add - link queue cannot be full (count %u must be less than size %u).", this->count, this->queue.size());

		uint index = (this->ptr + this->count) % this->queue.size();

		this->queue[index] = packet;
		this->time[index] = time;
		this->count++;
		this->usage += packet->Size();
	}

	void LinkQueue::Remove()
	{
		assert(this->count > 0);

		this->usage -= this->queue[this->ptr]->Size();
		this->ptr = (this->ptr + 1) % this->queue.size();
		this->count--;
	}
}