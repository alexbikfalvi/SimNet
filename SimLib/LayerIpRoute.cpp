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
#include "LayerIpRoute.h"

namespace SimLib
{
	LayerIpRoute::LayerIpRoute(
		SimHandler&	sim,
		AddressIpv4	address,
		Route&		route
		) : Layer(sim), address(address), route(route)
	{
		// Delegate and event
		this->delegateRecv = alloc TDelegateRecv(*this, &LayerIpRoute::Recv);
		this->eventSend = alloc Event2<void, uint, PacketIpv4*>();
	}

	LayerIpRoute::~LayerIpRoute()
	{
		delete this->delegateRecv;
		delete this->eventSend;
	}

	void LayerIpRoute::Recv(uint entry, PacketIpv4* packet)
	{
		// Forward all unicast packets that are not address to local address and have a TTL greater than zero
		if(packet->Ttl() == 0) return;
		if(packet->Dst().IsMulticast()) return;
		if(packet->Dst() == this->address) return;

		uint link = this->route.Forward(this->address, packet->Dst());

		(*this->eventSend)(link, type_cast<PacketIpv4*>(packet->Copy()));
	}
}
