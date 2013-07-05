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

#include "Layer.h"
#include "Delegate.h"
#include "Event.h"
#include "Route.h"
#include "PacketIpv4.h"

namespace SimLib
{
	class LayerIpRoute : public Layer
	{
	public:
		typedef Delegate2<LayerIpRoute, void, uint, PacketIpv4*>	TDelegateRecv;
		typedef IDelegate2<void, uint, PacketIpv4*>					TIDelegateRecv;
		typedef Event2<void, uint, PacketIpv4*>						TEventSend;

	private:
		AddressIpv4		address;
		Route&			route;

		// Delegates
		TDelegateRecv*	delegateRecv;

		// Events
		TEventSend*		eventSend;

	public:
		LayerIpRoute(
			SimHandler&	sim,
			AddressIpv4	address,
			Route&		route
			);
		virtual ~LayerIpRoute();

		inline TIDelegateRecv*	DelegateRecv() { return this->delegateRecv; }
		inline TEventSend*		EventSend() { return this->eventSend; }

		virtual void			Initialize() { }
		virtual void			Finalize() { }

	private:
		void					Recv(uint entry, PacketIpv4* packet);
	};
}
