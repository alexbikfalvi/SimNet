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
#include "PacketIpv4.h"
#include "Route.h"
#include "Delegate.h"
#include "Event.h"

namespace SimLib
{
	class LayerIpLocal : public Layer
	{
	public:
		typedef Delegate2<LayerIpLocal, void, uint, PacketIpv4*>			TDelegateRecv;
		typedef Delegate3<LayerIpLocal, void, AddressIpv4, __byte, Packet*>	TDelegateSend;

		typedef IDelegate2<void, uint, PacketIpv4*>							TIDelegateRecv;
		typedef IDelegate3<void, AddressIpv4, __byte, Packet*>				TIDelegateSend;

		typedef Event3<void, uint, PacketIpv4*, Packet*>					TEventRecv;
		typedef Event2<void, uint, PacketIpv4*>								TEventSend;
		typedef Event1<void, PacketIpv4*>									TEventSendMcast;

	private:
		AddressIpv4			address;
		Route&				route;

		TDelegateRecv*		delegateRecv;
		TDelegateRecv*		delegateRecvMcast;
		TEventRecv*			eventRecv;

		TDelegateSend*		delegateSend;
		TEventSend*			eventSend;
		TEventSendMcast*	eventSendMcast;

	public:
		LayerIpLocal(
			SimHandler&	sim,
			AddressIpv4	address,
			Route&		route
			);
		virtual ~LayerIpLocal();

		inline TIDelegateRecv*	DelegateRecv() { return this->delegateRecv; }
		inline TIDelegateRecv*	DelegateRecvMcast() { return this->delegateRecvMcast; }
		inline TEventRecv*		EventRecv() { return this->eventRecv; }

		inline TIDelegateSend*	DelegateSend() { return this->delegateSend; }
		inline TEventSend*		EventSend() { return this->eventSend; }
		inline TEventSendMcast*	EventSendMcast() { return this->eventSendMcast; }

		virtual void			Initialize() { }
		virtual void			Finalize() { }

	private:
		void					Recv(uint entry, PacketIpv4* packet);
		void					RecvMcast(uint entry, PacketIpv4* packet);
		void					Send(AddressIpv4 dst, __byte ttl, Packet* payload);
	};
}
