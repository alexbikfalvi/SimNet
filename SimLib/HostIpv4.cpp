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
#include "HostIpv4.h"
#include "RouteHost.h"

namespace SimLib
{
	HostIpv4::HostIpv4(
		uint		id,
		SimHandler&	sim,
		AddressIpv4	address
		) : NodeIpv4(id, sim, address, 1, RouteHost::route)
	{
		// Initialize layers
		this->layerIgmp = alloc LayerIgmp(this->sim, this->address);

		// Create layer connections
			// IP layer <-> IGMP layer
		(*this->layerIp->EventRecv()) += *this->layerIgmp->DelegateRecv();
		(*this->layerIgmp->EventSend()) += *this->layerIp->DelegateSend();
			// IGMP layer <-> IP multicast layer
		(*this->layerIgmp->EventJoin()) += *this->layerIpMcast->DelegateJoin();
		(*this->layerIgmp->EventLeave()) += *this->layerIpMcast->DelegateLeave();
		(*this->layerIgmp->EventLocalJoin()) += *this->layerIpMcast->DelegateLocalJoin();
		(*this->layerIgmp->EventLocalLeave()) += *this->layerIpMcast->DelegateLocalLeave();
	}

	HostIpv4::~HostIpv4()
	{
		// Delete layers
		delete this->layerIgmp;
	}

	void HostIpv4::Initialize()
	{
		// Call the base class initializer
		Node::Initialize();

		// Call the initializer for the layers
		this->layerIgmp->Initialize();
	}

	void HostIpv4::Finalize()
	{
		// Call the base class finalizer
		Node::Finalize();

		// Call the finalizer for the layers
		this->layerIgmp->Finalize();
	}
}
