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
#include "LayerUdp.h"

namespace SimLib
{
	LayerUdp::LayerUdp(SimHandler& sim) : Layer(sim)
	{
		this->delegateRecv = alloc TDelegateRecv(*this, &LayerUdp::Recv);
		this->eventRecv = alloc TEventRecv();
		this->delegateSend = alloc TDelegateSend(*this, &LayerUdp::Send);
		this->eventSend = alloc TEventSend();
	}

	LayerUdp::~LayerUdp()
	{
		delete this->delegateRecv;
		delete this->eventRecv;

		delete this->delegateSend;
		delete this->eventSend;
	}

	void LayerUdp::Recv(uint entry, PacketIpv4* ip, Packet* packet)
	{
		// Verify the packet is IP
		if(NULL == packet) return;
		if(packet->Type() != PACKET_TYPE_UDP) return;

		// Send the packet to the upper layers
		(*this->eventRecv)(entry, ip, type_cast<PacketUdp*>(packet), packet->Payload());
	}

	void LayerUdp::Send(__uint16 src, __uint16 dst, AddressIpv4 address, __byte ttl, Packet* payload)
	{
		// Create a alloc UDP packet
		PacketUdp* packet = alloc PacketUdp(src, dst, payload);

		// Send the packet to the lower layer
		(*this->eventSend)(address, ttl, packet);
	}
}
