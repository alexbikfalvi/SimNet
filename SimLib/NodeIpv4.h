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

#pragma once

#include "Node.h"
#include "AddressIpv4.h"
#include "Route.h"

#include "LayerIp.h"
#include "LayerIpLocal.h"
#include "LayerIpMcast.h"
#include "LayerIgmp.h"
#include "LayerUdp.h"

namespace SimLib
{
	class NodeIpv4 : public Node
	{
	protected:
		// Global
		AddressIpv4				address;
		Route&					route;

		// Layers
		LayerIp*				layerIp;
		LayerIpLocal*			layerIpLocal;
		LayerIpMcast*			layerIpMcast;
		LayerUdp*				layerUdp;

	public:
		NodeIpv4(
			uint			id,
			SimHandler&		sim,
			AddressIpv4		address,
			uint			numLinks,
			Route&			route
			);
		virtual ~NodeIpv4();

		// Global
		inline AddressIpv4		Address() { return this->address; }

		// Finalizer
		virtual void			Initialize();
		virtual void			Finalize();
	};
}