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
#include "RouterIpv4.h"

namespace SimLib
{
	RouterIpv4::RouterIpv4(
		uint						id,
		SimHandler&					sim,
		AddressIpv4					address,
		uint						numLinks,
		Route&						route,
		std::vector<AddressIpv4>&	rp
		) : NodeIpv4(id, sim, address, numLinks, route)
	{
		// Initialize layers
		this->layerIgmp = alloc LayerIgmp(this->sim, this->address, LayerIgmp::IGMP_ROUTER);
		this->layerIpRoute = alloc LayerIpRoute(
			this->sim,
			this->address,
			this->route
			);
		this->layerPimSm = alloc LayerPimSm(
			this->sim,
			this->address,
			rp,
			this->route
			);

		// Create layer connections
			// IP layer <-> IGMP layer
		(*this->layerIp->EventRecv()) += *this->layerIgmp->DelegateRecv();
		(*this->layerIgmp->EventSend()) += *this->layerIp->DelegateSend();
			// IGMP layer <-> IP multicast layer
		(*this->layerIgmp->EventJoin()) += *this->layerIpMcast->DelegateJoin();
		(*this->layerIgmp->EventLeave()) += *this->layerIpMcast->DelegateLeave();
		(*this->layerIgmp->EventLocalJoin()) += *this->layerIpMcast->DelegateLocalJoin();
		(*this->layerIgmp->EventLocalLeave()) += *this->layerIpMcast->DelegateLocalLeave();
			// Layer IP <-> Layer IP route
		(*this->layerIp->EventRecv()) += *this->layerIpRoute->DelegateRecv();
		(*this->layerIpRoute->EventSend()) += *this->layerIp->DelegateSend();
			// Layer IGMP  <-> Layer PIM-SM
		(*this->layerIgmp->EventJoin()) += *this->layerPimSm->DelegateJoin();
		(*this->layerIgmp->EventLeave()) += *this->layerPimSm->DelegateLeave();
			// Layer IP <-> Layer PIM-SM
		(*this->layerIp->EventRecv()) += *this->layerPimSm->DelegateRecv();
		(*this->layerPimSm->EventSend()) += *this->layerIp->DelegateSend();
			// Layer PIM-SM <-> Layer IP multicast
		(*this->layerPimSm->EventJoin()) += *this->layerIpMcast->DelegateJoin();
		(*this->layerPimSm->EventLeave()) += *this->layerIpMcast->DelegateLeave();
	}

	RouterIpv4::~RouterIpv4()
	{
		// Delete layers
		delete this->layerIgmp;
		delete this->layerIpRoute;
		delete this->layerPimSm;
	}

	void RouterIpv4::Initialize()
	{
		// Call base class initializer
		NodeIpv4::Initialize();

		// Call the initializer for each layer
		this->layerIgmp->Initialize();
		this->layerIpRoute->Initialize();
		this->layerPimSm->Initialize();
	}

	void RouterIpv4::Finalize()
	{
		// Call base class finalizer
		NodeIpv4::Finalize();

		// Call the finalizer for each layer
		this->layerIgmp->Finalize();
		this->layerIpRoute->Finalize();
		this->layerPimSm->Finalize();
	}
}