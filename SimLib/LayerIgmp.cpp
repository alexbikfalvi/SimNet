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
#include "LayerIgmp.h"
#include "ExceptionArgument.h"

namespace SimLib
{
	LayerIgmp::LayerIgmp(
		SimHandler&	sim,
		AddressIpv4	address,
		EIgmpType	type
		) : Layer(sim), address(address), type(type)
	{
		// Delegates
		this->delegateRecv = alloc TDelegateRecv(*this, &LayerIgmp::Recv);
		this->delegateJoin = alloc TDelegateJoin(*this, &LayerIgmp::Join);
		this->delegateLeave = alloc TDelegateLeave(*this, &LayerIgmp::Leave);

		// Events
		this->eventJoin = alloc TEventJoin();
		this->eventLeave = alloc TEventLeave();
		this->eventLocalJoin = alloc TEventLocalJoin();
		this->eventLocalLeave = alloc TEventLocalLeave();
		this->eventSend = alloc TEventSend();

		// Statistics
		this->statEntries = 0;
		this->statEntriesNum = 0;
		this->statEntriesLast = 0;
	}

	LayerIgmp::~LayerIgmp()
	{
		delete this->delegateRecv;
		delete this->delegateJoin;
		delete this->delegateLeave;

		delete this->eventJoin;
		delete this->eventLeave;
	
		delete this->eventLocalJoin;
		delete this->eventLocalLeave;

		delete this->eventSend;
	}

	void LayerIgmp::Join(AddressIpv4 address, uint entry)
	{
		// Check address is multicast
		if(!address.IsMulticast()) throw ExceptionArgument(__FILE__, __LINE__, "LayerIgmp::Join - address %x is not multicast", address.Addr()); 
		
		// Get the multicast group
		uint group = address.MulticastGroup();

		// If the host is part of the multicast group, return
		if(this->memberships[group].State() == LayerIpMcastMembership::MEMBER) return;

		// Send an unsolicited membership report to the IP multicast address of the group
		PacketIgmp* packet = alloc PacketIgmp(address, PacketIgmp::PACKET_IGMP_MEMBERSHIP_REPORT);

		// Send the IGMP packet on the joined interface, group multicast address and TTL of 1
		this->Send(entry, address, 1, packet);
	
		// Set the group state to joined
		this->memberships[group].Join(entry);

		// Call event for membership change
		(*this->eventLocalJoin)(entry, address);
	}

	void LayerIgmp::Leave(AddressIpv4 address)
	{
		// Check address is multicast
		if(!address.IsMulticast()) throw ExceptionArgument(__FILE__, __LINE__, "LayerIgmp::Leave - address %x is not multicast", address.Addr()); 

		// Get the multicast group
		uint group = address.MulticastGroup();

		// If the host is not part of the multicast group, return
		if(this->memberships[group].State() == LayerIpMcastMembership::NON_MEMBER) return;

		// Send a membership leave to the all routers multicast address
		PacketIgmp* packet = alloc PacketIgmp(address, PacketIgmp::PACKET_IGMP_LEAVE_GROUP);

		// Send the IGMP packet on the joined interface, all routers multicast address and TTL of 1
		this->Send(this->memberships[group].Entry(), AddressIpv4::MCAST_ALL_ROUTERS, 1, packet);

		// Call event for membership change
		(*this->eventLocalLeave)(this->memberships[group].Entry(), address);

		// Set group state to leave
		this->memberships[group].Leave();
	}

	void LayerIgmp::Recv(uint entry, PacketIpv4* packet)
	{
		// Process only IGMP packet
		if(packet->Payload() == NULL) return;
		if(packet->Payload()->Type() != PACKET_TYPE_IGMP) return;

		switch(packet->Dst().Addr())
		{
		case AddressIpv4::MCAST_ALL_SYSTEMS: this->RecvIgmpAllSystems(entry, packet->Src(), packet->Dst(), type_cast<PacketIgmp*>(packet->Payload())); break;
		case AddressIpv4::MCAST_ALL_ROUTERS: this->RecvIgmpAllRouters(entry, packet->Src(), packet->Dst(), type_cast<PacketIgmp*>(packet->Payload())); break;
		default: this->RecvIgmpOther(entry, packet->Src(), packet->Dst(), type_cast<PacketIgmp*>(packet->Payload())); break;
		}
	}

	void LayerIgmp::RecvIgmpAllSystems(uint entry, AddressIpv4 src, AddressIpv4 dst, PacketIgmp* packet)
	{
		// Do nothing (current implementation does not process all systems IGMP packets)
	}

	void LayerIgmp::RecvIgmpAllRouters(uint entry, AddressIpv4 src, AddressIpv4 dst, PacketIgmp* packet)
	{
		// Process all routers IGMP packets if the IGMP type is router
		if(this->type != IGMP_ROUTER) return;

		switch(packet->IgmpType())
		{
		case PacketIgmp::PACKET_IGMP_LEAVE_GROUP: this->RecvIgmpLeaveGroup(entry, src, dst, packet); break; // Leave group
		default:; // Do nothing
		}
	}

	void LayerIgmp::RecvIgmpOther(uint entry, AddressIpv4 src, AddressIpv4 dst, PacketIgmp* packet)
	{
		// Process only multicast addresses or unicast addressed to this host
		if((!dst.IsMulticast()) && (dst != this->address)) return;

		switch(packet->IgmpType())
		{
		case PacketIgmp::PACKET_IGMP_MEMBERSHIP_REPORT: this->RecvIgmpJoinGroup(entry, src, dst, packet); break; // Join group
		default:; // Do nothing
		}
	}

	void LayerIgmp::RecvIgmpJoinGroup(uint entry, AddressIpv4 src, AddressIpv4 dst, PacketIgmp* packet)
	{
		// Join group (membership report)
		uint group = dst.MulticastGroup();

		// Statistics
		this->statEntries += (this->sim.Time() - this->statEntriesLast) * this->statEntriesNum;
		this->statEntriesNum -= this->groups[group].Out()->size();

		// Add the interface to the group
		this->groups[group].Out()->insert(entry);

		// Statistics
		this->statEntriesNum += this->groups[group].Out()->size();
		this->statEntriesLast = this->sim.Time();

		// Call event for this group
		(*this->eventJoin)(entry, dst, &this->groups[group]);
	}

	void LayerIgmp::RecvIgmpLeaveGroup(uint entry, AddressIpv4 src, AddressIpv4 dst, PacketIgmp* packet)
	{
		// Leave group
		uint group = packet->Group().MulticastGroup();

		// Statistics
		this->statEntries += (this->sim.Time() - this->statEntriesLast) * this->statEntriesNum;
		this->statEntriesNum -= this->groups[group].Out()->size();

		// Remove the interface from the group
		this->groups[group].Out()->erase(entry);

		// Statistics
		this->statEntriesNum += this->groups[group].Out()->size();
		this->statEntriesLast = this->sim.Time();

		// Call event for this group
		(*this->eventLeave)(entry, packet->Group(), &this->groups[group]);
	}

	void LayerIgmp::Send(uint entry, AddressIpv4 dst, __byte ttl, Packet* payload)
	{
		// Create a alloc IP packet
		PacketIpv4* packet = alloc PacketIpv4(this->address, dst, ttl, payload);

		// Send the packet to the lower layer
		(*this->eventSend)(entry, packet);
	}

	void LayerIgmp::Initialize()
	{
		this->memberships.clear();
		this->groups.clear();
	}

	void LayerIgmp::Finalize()
	{
		this->statEntries += (this->sim.Time() - this->statEntriesLast) * this->statEntriesNum;
		this->statEntries /= this->sim.Time();
	}
}