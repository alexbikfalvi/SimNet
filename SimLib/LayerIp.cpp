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
#include "LayerIp.h"

namespace SimLib
{
	LayerIp::LayerIp(SimHandler& sim) : Layer(sim)
	{
		this->delegateRecv = alloc TDelegateRecv(*this, &LayerIp::Recv);
		this->eventRecv = alloc TEventRecv();
		
		this->delegateSend = alloc TDelegateSend(*this, &LayerIp::Send);
		this->eventSend = alloc TEventSend();
	}

	LayerIp::~LayerIp()
	{
		delete this->delegateRecv;
		delete this->eventRecv;

		delete this->delegateSend;
		delete this->eventSend;
	}

	void LayerIp::Recv(uint entry, Packet* packet)
	{
		// Verify the packet is IP
		if(NULL == packet) return;
		if(packet->Type() != PACKET_TYPE_IP) return;

		// Get the IP packet
		PacketIpv4* ip = type_cast<PacketIpv4*>(packet);

		// Decrement the TTL
		ip->DecTtl();

		// Send the packet to the upper layers
		(*this->eventRecv)(entry, ip);
	}

	void LayerIp::Send(uint entry, PacketIpv4* packet)
	{
		if(NULL == packet) return;

		// Send the packer to the lower layer
		(*this->eventSend)(entry, packet);
	}
}
