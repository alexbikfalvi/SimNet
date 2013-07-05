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
#include "LayerPimSm.h"
#include "ExceptionArgument.h"
#include "ExceptionUnsupported.h"

namespace SimLib
{
	LayerPimSm::LayerPimSm(
		SimHandler&					sim,
		AddressIpv4					address,
		std::vector<AddressIpv4>&	rp,
		Route&						route
		) : Layer(sim), address(address), route(route), rp(rp)
	{
		// Delegates
		this->delegateRecv = alloc TDelegateRecv(*this, &LayerPimSm::Recv);
		this->delegateJoin = alloc TDelegateOp(*this, &LayerPimSm::Join);
		this->delegateLeave = alloc TDelegateOp(*this, &LayerPimSm::Leave);

		// Events
		this->eventJoin = alloc TEventOp();
		this->eventLeave = alloc TEventOp();
		this->eventSend = alloc TEventSend();

		// Statistics
		this->statEntries = 0;
		this->statEntriesNum = 0;
		this->statEntriesLast = 0;
	}

	LayerPimSm::~LayerPimSm()
	{
		// Delegates
		delete this->delegateRecv;
		delete this->delegateJoin;
		delete this->delegateLeave;

		// Events
		delete this->eventJoin;
		delete this->eventLeave;
		delete this->eventSend;
	}

	void LayerPimSm::Recv(uint entry, PacketIpv4* packet)
	{
		// Process only PIM-SM packets
		if(packet->Dst() != AddressIpv4::MCAST_PIM_SM) return;
		if(packet->Payload() == NULL) return;
		if(packet->Payload()->Type() != PACKET_TYPE_PIM_SM) return;

		this->RecvPimSm(entry, packet->Src(), packet->Dst(), type_cast<PacketPimSm*>(packet->Payload()));
	}

	void LayerPimSm::RecvPimSm(uint entry, AddressIpv4 src, AddressIpv4 dst, PacketPimSm* packet)
	{
		switch(packet->PimSmType())
		{
		case PacketPimSm::JOIN_PRUNE: this->RecvPimSmJoinPrune(entry, src, dst, type_cast<PacketPimSmJoinPrune*>(packet)); break;
		default: throw ExceptionUnsupported(__FILE__, __LINE__, "LayerPimSm::RecvPimSm - packet PIM-SM type %u is not supported.", packet->PimSmType()); // not supported
		}
	}

	void LayerPimSm::RecvPimSmJoinPrune(uint entry, AddressIpv4 src, AddressIpv4 dst, PacketPimSmJoinPrune* packet)
	{
		// Check the address is multicast
		if(!packet->Address().IsMulticast()) throw ExceptionArgument(__FILE__, __LINE__, "LayerPimSm::RecvPimSmJoinPrune - address %x is not multicast", packet->Address().Addr()); 
		
		// Get the group
		uint group = packet->Address().MulticastGroup();

		switch(packet->JoinPruneType())
		{
		case PacketPimSmJoinPrune::JOIN:
			// Join the group
			this->JoinGroup(group, packet->Address());

			// Statistics
			this->statEntries += (this->sim.Time() - this->statEntriesLast) * this->statEntriesNum;
			this->statEntriesNum -= this->groups[group].Out()->size();

			// Add the interface to the list of outbound interfaces for this group
			this->groups[group].Out()->insert(entry);

			// Statistics
			this->statEntriesNum += this->groups[group].Out()->size();
			this->statEntriesLast = this->sim.Time();

			// Call event for this group
			(*this->eventJoin)(entry, packet->Address(), &this->groups[group]);
		
			break;
		case PacketPimSmJoinPrune::PRUNE:
			// Statistics
			this->statEntries += (this->sim.Time() - this->statEntriesLast) * this->statEntriesNum;
			this->statEntriesNum -= this->groups[group].Out()->size();

			// Remove the interface from the list of outbound interfaces for this group
			this->groups[group].Out()->erase(entry);

			// Statistics
			this->statEntriesNum += this->groups[group].Out()->size();
			this->statEntriesLast = this->sim.Time();
	
			// Leave the group
			this->LeaveGroup(group, packet->Address());

			// Call event for this group
			(*this->eventLeave)(entry, packet->Address(), &this->groups[group]);

			break;
		default: throw ExceptionUnsupported(__FILE__, __LINE__, "LayerPimSm::RecvPimSmJoinPrune - join/prune type %u is not supported", packet->JoinPruneType());
		}
	}

	void LayerPimSm::Join(uint entry, AddressIpv4 address, LayerIpMcastGroup* igmp)
	{
		// Check the address is multicast
		if(!address.IsMulticast()) throw ExceptionArgument(__FILE__, __LINE__, "LayerPimSm::Join - address %x is not multicast", address.Addr()); 

		uint group = address.MulticastGroup();

		// Join the group
		this->JoinGroup(group, address);

		// Update group membership
		this->memberships[group] = igmp->Out()->size()?LayerPimSmMembership::MEMBER:LayerPimSmMembership::NON_MEMBER;
	}

	void LayerPimSm::Leave(uint entry, AddressIpv4 address, LayerIpMcastGroup* igmp)
	{
		// Check the address is multicast
		if(!address.IsMulticast()) throw ExceptionArgument(__FILE__, __LINE__, "LayerPimSm::Leave - address %x is not multicast", address.Addr()); 

		uint group = address.MulticastGroup();

		// Update group membership
		this->memberships[group] = igmp->Out()->size()?LayerPimSmMembership::MEMBER:LayerPimSmMembership::NON_MEMBER;

		// Leave the group
		this->LeaveGroup(group, address);
	}

	void LayerPimSm::JoinGroup(uint group, AddressIpv4 address)
	{
		// If the group is alloc (i.e. no output interfaces) and this is not the RP router for this group and there is no (IGMP) membership
		if((this->groups[group].Out()->size() == 0) && (this->rp[group] != this->address) && (this->memberships[group].State() == LayerPimSmMembership::NON_MEMBER))
		{
			// Send a JOIN message toward the RP router
			PacketPimSmJoinPrune* join = alloc PacketPimSmJoinPrune(
				PacketPimSmJoinPrune::JOIN,
				address
				);

			// Send the packet on the link toward the RP with TTL of 1
			uint link = this->route.Forward(this->address, this->rp[group]);

			this->Send(link, AddressIpv4::MCAST_PIM_SM, 1, join);
		}
	}

	void LayerPimSm::LeaveGroup(uint group, AddressIpv4 address)
	{
		// If the group is empty (i.e. no output interfaces) and this is not the RP router for this group and there are no memberships (IGMP)
		if((this->groups[group].Out()->size() == 0) && (this->rp[group] != this->address) && (this->memberships[group].State() == LayerPimSmMembership::NON_MEMBER))
		{
			// Send a PRUNE message toward the RP router
			PacketPimSmJoinPrune* prune = alloc PacketPimSmJoinPrune(
				PacketPimSmJoinPrune::PRUNE,
				address
				);

			// Send the packet on the link toward the RP with TTL of 1
			uint link = this->route.Forward(this->address, this->rp[group]);

			this->Send(link, AddressIpv4::MCAST_PIM_SM, 1, prune);
		}
	}

	void LayerPimSm::Send(uint entry, AddressIpv4 dst, __byte ttl, Packet* payload)
	{
		// Create a alloc IP packet
		PacketIpv4* packet = alloc PacketIpv4(this->address, dst, ttl, payload);

		// Send the packet to the lower layer
		(*this->eventSend)(entry, packet);
	}

	void LayerPimSm::Initialize()
	{
		this->memberships.clear();
		this->groups.clear();
	}

	void LayerPimSm::Finalize()
	{
		this->statEntries += (this->sim.Time() - this->statEntriesLast) * this->statEntriesNum;
		this->statEntries /= this->sim.Time();
	}
}
