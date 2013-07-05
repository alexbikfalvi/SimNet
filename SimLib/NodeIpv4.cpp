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
#include "NodeIpv4.h"

namespace SimLib
{
	NodeIpv4::NodeIpv4(
		uint		id,
		SimHandler&	sim,
		AddressIpv4	address,
		uint		numLinks,
		Route&		route
		) : Node(id, sim, numLinks), address(address), route(route)
	{
		// Initialize layers
		this->layerIp = alloc LayerIp(this->sim);
		this->layerIpLocal = alloc LayerIpLocal(this->sim, this->address, this->route);
		this->layerIpMcast = alloc LayerIpMcast(this->sim);
		this->layerUdp = alloc LayerUdp(this->sim);

		// Create layer connections
			// Base layer <-> IP layer
		(*this->eventRecv) += *this->layerIp->DelegateRecv();
		(*this->layerIp->EventSend()) += *this->delegateSend;
			// IP layer <-> IP local layer
		(*this->layerIp->EventRecv()) += *this->layerIpLocal->DelegateRecv();
		(*this->layerIpLocal->EventSend()) += *this->layerIp->DelegateSend();
			// IP local layer <-> UDP layer
		(*this->layerIpLocal->EventRecv()) += *this->layerUdp->DelegateRecv();
		(*this->layerUdp->EventSend()) += *this->layerIpLocal->DelegateSend();
			// IP layer <-> IP multicast layer
		(*this->layerIp->EventRecv()) += *this->layerIpMcast->DelegateRecv();
		(*this->layerIpMcast->EventSend()) += *this->layerIp->DelegateSend();
			// IP local layer <-> IP multicast layer
		(*this->layerIpMcast->EventRecv()) += *this->layerIpLocal->DelegateRecvMcast();
		(*this->layerIpLocal->EventSendMcast()) += *this->layerIpMcast->DelegateSend();

		// Statistics
		this->statPacketsRead = 0;
		this->statPacketsWrite = 0;

		this->statDataRead = 0;
		this->statDataWrite = 0;
	}

	NodeIpv4::~NodeIpv4()
	{
		// Delete layers
		delete this->layerIp;
		delete this->layerIpLocal;
		delete this->layerIpMcast;
		delete this->layerUdp;
	}

	void NodeIpv4::Initialize()
	{
		// Call the base class initializer
		Node::Initialize();

		// Call the initializers for all layers
		this->layerIp->Initialize();
		this->layerIpLocal->Initialize();
		this->layerIpMcast->Initialize();
		this->layerUdp->Initialize();
	}

	void NodeIpv4::Finalize()
	{
		// Call the base class finalizer
		Node::Finalize();

		// Call the finalizer for all layers
		this->layerIp->Finalize();
		this->layerIpLocal->Finalize();
		this->layerIpMcast->Finalize();
		this->layerUdp->Finalize();
	}
}