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
#include "Link.h"
#include "Delegate.h"
#include "Event.h"

namespace SimLib
{
	class Node : public Object
	{
	public:
		typedef Delegate2<Node, void, uint, Packet*>	TDelegateSend;
		typedef Event2<void, uint, Packet*>				TEventRecv;

	protected:
		// Global
		uint					id;

		// Links
		uint					currentLink;
	
		std::vector<Link*>		links;
		std::vector<uint>		linkEntries;
		std::map<uint, uint>	linkIndices;

		// Stat
		__uint64				statPacketsRead;
		__uint64				statPacketsWrite;

		__uint64				statDataRead;
		__uint64				statDataWrite;

		// Base layer delegate
		TDelegateSend*			delegateSend;

		// Base layer event
		TEventRecv*				eventRecv;

	public:
		Node(
			uint			id,
			SimHandler&		sim,
			uint			numLinks
			);
		virtual ~Node();

		// Global
		inline uint				Id() { return this->id; }

		// Links
		void					AddLink(Link* link);

		inline uint				NumLinks() { assert(this->links.size() == this->currentLink); return this->links.size(); }
		inline Link*			Link(uint index) { assert(index < this->currentLink); return this->links[index]; }
		inline uint				LinkEntry(uint index) { assert(index < this->currentLink); return this->linkEntries[index]; }
		inline uint				LinkIndex(uint id) { assert(this->linkIndices.find(id) != this->linkIndices.end()); return this->linkIndices.find(id)->second; }

		// Receive
		virtual ERecvCode		Recv(Object* sender, uint entry, Packet* packet);

		// Stat
		inline __uint64			StatPacketsRead() { return this->statPacketsRead; }
		inline __uint64			StatPacketsWrite() { return this->statPacketsWrite; }

		inline __uint64			StatDataRead() { return this->statDataRead; }
		inline __uint64			StatDataWrite() { return this->statDataWrite; }

		virtual void			Initialize() { }
		virtual void			Finalize() { }

	private:
		void					Send(uint entry, Packet* packet);
	};
}