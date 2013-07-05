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

#define LINK_QUEUE_NEXT(ptr) ((ptr + 1) % this->size)

namespace SimLib
{
	class LinkQueue
	{
	protected:
		std::vector<Packet*>	queue;
		std::vector<__time>		time;
		uint					ptr;
		uint					count;
		__bits					usage;

	public:
		LinkQueue(
			uint	size
			);
		~LinkQueue();

		inline bool			IsEmpty() { return this->count == 0; }
		inline bool			IsFull() { return this->count == this->queue.size(); }

		inline Packet*		First() { return this->count?this->queue[this->ptr]:NULL; }
		inline Packet*		Last() { return this->count?this->queue[(this->ptr + this->count - 1) % this->queue.size()]:NULL; }

		inline uint			Size() { return this->queue.size(); }
		inline uint			Packets() { return this->count; }
		inline __bits		Usage() {  return this->usage; }

		inline __time		TimeFirst() { return this->time[this->ptr]; }

		void				Add(__time time, Packet* packet);
		void				Remove();
	};
}