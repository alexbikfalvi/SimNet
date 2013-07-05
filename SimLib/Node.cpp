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
#include "Node.h"
#include "ExceptionArgument.h"
#include "ExceptionUnsupported.h"

namespace SimLib
{
	Node::Node(
		uint		id,
		SimHandler&	sim,
		uint		numLinks
		) : Object(sim), id(id)
	{
		// Links
		this->currentLink = 0;
		this->links.resize(numLinks);
		this->linkEntries.resize(numLinks);

		// Base layer delegate and event
		this->delegateSend = alloc TDelegateSend(*this, &Node::Send);
		this->eventRecv = alloc TEventRecv();

		// Statistics
		this->statPacketsRead = 0;
		this->statPacketsWrite = 0;

		this->statDataRead = 0;
		this->statDataWrite = 0;
	}

	Node::~Node()
	{
		// Delete base delegate and event
		delete this->delegateSend;
		delete this->eventRecv;
	}

	void Node::AddLink(SimLib::Link* link)
	{
		if(NULL == link) throw ExceptionArgument(__FILE__, __LINE__, "Node::AddLink - link cannot be null.");
		if(this->currentLink >= this->links.size()) throw ExceptionUnsupported(__FILE__, __LINE__, "Node::AddLink - links vector is full (current link is %u, and links size is %u).", this->currentLink, this->links.size());

		// Add link to the node
		this->links[this->currentLink] = link;
	
		// Add node to the link
		this->linkEntries[this->currentLink] = link->AddNode(this, this->currentLink);

		// Add link index
		this->linkIndices.insert(std::pair<uint, uint>(link->Id(), this->currentLink));

		this->currentLink++;
	}

	Object::ERecvCode Node::Recv(Object* sender, uint entry, Packet* packet)
	{
		if(NULL == sender) throw ExceptionArgument(__FILE__, __LINE__, "Node::Recv - sender cannot be null.");
		if(NULL == packet) throw ExceptionArgument(__FILE__, __LINE__, "Node::Recv - packet cannot be null.");
		if(sender != this->links[entry]) throw ExceptionUnsupported(__FILE__, __LINE__, "Node::Recv - sender %p different from link %p at entry %u", sender, this->links[entry], entry);

		// Send the packet to the upper layers
		(*this->eventRecv)(entry, packet);

		// Stat
		this->statPacketsRead++;
		this->statDataRead += packet->Size();

		// Delete the packet
		packet->Delete();
		delete packet;

		return RECV_SUCCESS;
	}

	void Node::Send(uint entry, Packet* packet)
	{
		if(NULL == packet) throw ExceptionArgument(__FILE__, __LINE__, "Node::Send - packet cannot be null.");
		if(entry >= this->links.size()) throw ExceptionUnsupported(__FILE__, __LINE__, "Node::Send - entry %u greater than the number of links %u.", entry, this->links.size());

		this->statPacketsWrite++;
		this->statDataWrite += packet->Size();

		this->links[entry]->Recv(this, this->linkEntries[entry], packet);
	}
}