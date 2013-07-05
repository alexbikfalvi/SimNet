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
#include "LayerIpMcast.h"
#include "ExceptionArgument.h"

namespace SimLib
{
	LayerIpMcast::LayerIpMcast(
		SimHandler&	sim
		) : Layer(sim)
	{
		// Delegates
		this->delegateRecv = alloc TDelegateRecv(*this, &LayerIpMcast::Recv);
		this->delegateSend = alloc TDelegateSend(*this, &LayerIpMcast::Send);
		this->delegateJoin =  alloc TDelegateOp(*this, &LayerIpMcast::Join);
		this->delegateLeave = alloc TDelegateOp(*this, &LayerIpMcast::Leave);
		this->delegateLocalJoin = alloc TDelegateLocalOp(*this, &LayerIpMcast::LocalJoin);
		this->delegateLocalLeave = alloc TDelegateLocalOp(*this, &LayerIpMcast::LocalLeave);

		// Events
		this->eventRecv = alloc TEventOp();
		this->eventSend = alloc TEventOp();
	}

	LayerIpMcast::~LayerIpMcast()
	{

		// Delegates
		delete this->delegateRecv;
		delete this->delegateSend;
		delete this->delegateJoin;
		delete this->delegateLeave;
		delete this->delegateLocalJoin;
		delete this->delegateLocalLeave;

		// Event
		delete this->eventRecv;
		delete this->eventSend;
	}

	void LayerIpMcast::Recv(uint entry, PacketIpv4* packet)
	{
		// Process only multicast packets addressed to a multicast group
		if(!packet->Dst().IsMulticast()) return;

		// Calculate the group
		uint group = packet->Dst().MulticastGroup();

		/*
		 * Local forwarding
		 */
		// If there is a local membership for this group send the packet payload to the upper layers
		if(this->memberships[group].State() == LayerIpMcastMembership::MEMBER)
		{
			(*this->eventRecv)(entry, packet);
		}

		/*
		 * Multicast routing
		 */
		// Execute multicast forwarding only if TTL is greater than zero
		if(packet->Ttl() == 0) return;

		// Forward a copy of the packet to all output interfaces for this group except the inbound interface
		for(LayerIpMcastGroup::Iterator iter = this->groups[group].Out()->begin(); iter != this->groups[group].Out()->end(); iter++)
		{
			if(*iter != entry)
			{
				(*this->eventSend)(*iter, type_cast<PacketIpv4*>(packet->Copy()));
			}
		}
	}

	void LayerIpMcast::Send(PacketIpv4* packet)
	{
		// Process only packets addressed to a multicast group
		if(!packet->Dst().IsMulticast()) return;

		// Get the group
		uint group = packet->Dst().MulticastGroup();

		// Forward the packet only if there is a membership for this group
		if(this->memberships[group].State() != LayerIpMcastMembership::MEMBER) return;

		// Send the packet to the lower layer on the membership interface
		(*this->eventSend)(this->memberships[group].Entry(), packet);
	}

	void LayerIpMcast::Join(uint entry, AddressIpv4 address, LayerIpMcastGroup* groupSender)
	{
		// Check the address is multicast
		if(!address.IsMulticast()) throw ExceptionArgument(__FILE__, __LINE__, "LayerIpMcast::Join - address %x is not multicast", address.Addr()); 

		uint group = address.MulticastGroup();

		// Add interface to multicast group
		this->groups[group].Out()->insert(entry);
	}

	void LayerIpMcast::Leave(uint entry, AddressIpv4 address, LayerIpMcastGroup* groupSender)
	{
		// Check the address is multicast
		if(!address.IsMulticast()) throw ExceptionArgument(__FILE__, __LINE__, "LayerIpMcast::Leave - address %x is not multicast", address.Addr()); 

		uint group = address.MulticastGroup();

		// Remove interface from multicast group
		this->groups[group].Out()->erase(entry);
	}

	void LayerIpMcast::LocalJoin(uint entry, AddressIpv4 address)
	{
		// Check the address is multicast
		if(!address.IsMulticast()) throw ExceptionArgument(__FILE__, __LINE__, "LayerIpMcast::LocalJoin - address %x is not multicast", address.Addr()); 

		uint group = address.MulticastGroup();

		// Add membership for this group and interface
		this->memberships[group].Join(entry);
	}

	void LayerIpMcast::LocalLeave(uint entry, AddressIpv4 address)
	{
		// Check the address is multicast
		if(!address.IsMulticast()) throw ExceptionArgument(__FILE__, __LINE__, "LayerIpMcast::LocalLeave - address %x is not multicast", address.Addr()); 

		uint group = address.MulticastGroup();

		// Remove membership for this group and interface
		this->memberships[group].Leave();
	}

	void LayerIpMcast::Initialize()
	{
		this->memberships.clear();
		this->groups.clear();
	}
}
