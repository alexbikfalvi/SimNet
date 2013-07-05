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
#include "LayerIpLocal.h"

namespace SimLib
{
	LayerIpLocal::LayerIpLocal(
		SimHandler&	sim,
		AddressIpv4	address,
		Route&		route
		) : Layer(sim), address(address), route(route)
	{
		// Delegates
		this->delegateRecv = alloc TDelegateRecv(*this, &LayerIpLocal::Recv);
		this->delegateRecvMcast = alloc TDelegateRecv(*this, &LayerIpLocal::RecvMcast);
		this->delegateSend = alloc TDelegateSend(*this, &LayerIpLocal::Send);

		// Events
		this->eventRecv = alloc TEventRecv();
		this->eventSend = alloc TEventSend();
		this->eventSendMcast = alloc TEventSendMcast();
	}

	LayerIpLocal::~LayerIpLocal()
	{
		// Delegates
		delete this->delegateRecv;
		delete this->delegateRecvMcast;
		delete this->delegateSend;

		// Events
		delete this->eventRecv;
		delete this->eventSend;
		delete this->eventSendMcast;
	}

	void LayerIpLocal::Recv(uint entry, PacketIpv4* packet)
	{
		// Verify the packet destination is the current address
		if(NULL == packet) return;
		if(packet->Dst() != this->address) return;

		// Send the packet payload to the upper layers
		(*this->eventRecv)(entry, packet, packet->Payload());
	}

	void LayerIpLocal::RecvMcast(uint entry, PacketIpv4* packet)
	{
		// Verify the packet destination is a multicast address
		if(NULL == packet) return;
		if(!packet->Dst().IsMulticast()) return;

		// Send the packet payload to the upper layers
		(*this->eventRecv)(entry, packet, packet->Payload());
	}

	void LayerIpLocal::Send(AddressIpv4 dst, __byte ttl, Packet *payload)
	{
		// Create a alloc IP packet
		PacketIpv4* packet = alloc PacketIpv4(this->address, dst, ttl, payload);

		// If destination is multicast
		if(dst.IsMulticast())
		{
			// Send the packet to the lower multicast layer
			(*this->eventSendMcast)(packet);
		}
		// If destination is unicast
		else
		{
			// Calculate the link to send the packet
			uint link = this->route.Forward(this->address, dst);

			// Send the packet to the lower layer
			(*this->eventSend)(link, packet);
		}
	}
 }