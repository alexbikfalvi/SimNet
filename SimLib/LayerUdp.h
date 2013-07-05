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
#include "PacketUdp.h"
#include "Delegate.h"
#include "Event.h"

namespace SimLib
{
	class LayerUdp : public Layer
	{
	public:
		typedef Delegate3<LayerUdp, void, uint, PacketIpv4*, Packet*>						TDelegateRecv;
		typedef Delegate5<LayerUdp, void, __uint16, __uint16, AddressIpv4, __byte, Packet*>	TDelegateSend;

		typedef IDelegate3<void, uint, PacketIpv4*, Packet*>								TIDelegateRecv;
		typedef IDelegate5<void, __uint16, __uint16, AddressIpv4, __byte, Packet*>			TIDelegateSend;

		typedef Event4<void, uint, PacketIpv4*, PacketUdp*, Packet*>						TEventRecv;
		typedef Event3<void, AddressIpv4, __byte, Packet*>									TEventSend;

	private:
		TDelegateRecv*	delegateRecv;
		TEventRecv*		eventRecv;

		TDelegateSend*	delegateSend;
		TEventSend*		eventSend;

	public:
		LayerUdp(SimHandler& sim);
		virtual ~LayerUdp();

		inline TIDelegateRecv*	DelegateRecv() { return this->delegateRecv; }
		inline TEventRecv*		EventRecv() { return this->eventRecv; }

		inline TIDelegateSend*	DelegateSend() { return this->delegateSend; }
		inline TEventSend*		EventSend() { return this->eventSend; }

		virtual void			Initialize() { }
		virtual void			Finalize() { }

	private:
		void					Recv(uint entry, PacketIpv4* ip, Packet* packet);
		void					Send(__uint16 src, __uint16 dst, AddressIpv4 address, __byte ttl, Packet* payload);
	};
}
