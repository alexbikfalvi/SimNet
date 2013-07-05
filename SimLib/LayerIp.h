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
#include "Delegate.h"
#include "Event.h"

namespace SimLib
{
	class LayerIp : public Layer
	{
	public:
		typedef Delegate2<LayerIp, void, uint, Packet*>		TDelegateRecv;
		typedef Event2<void, uint, PacketIpv4*>				TEventRecv;

		typedef Delegate2<LayerIp, void, uint, PacketIpv4*>	TDelegateSend;
		typedef Event2<void, uint, Packet*>					TEventSend;

		typedef IDelegate2<void, uint, Packet*>				TIDelegateRecv;
		typedef IDelegate2<void, uint, PacketIpv4*>			TIDelegateSend;

	private:
		TDelegateRecv*	delegateRecv;
		TEventRecv*		eventRecv;

		TDelegateSend*	delegateSend;
		TEventSend*		eventSend;

	public:
		LayerIp(SimHandler& sim);
		virtual ~LayerIp();

		inline TIDelegateRecv*	DelegateRecv() { return this->delegateRecv; }
		inline TEventRecv*		EventRecv() { return this->eventRecv; }

		inline TIDelegateSend*	DelegateSend() { return this->delegateSend; }
		inline TEventSend*		EventSend() { return this->eventSend; }

		virtual void			Initialize() { }
		virtual void			Finalize() { }

	private:
		void					Recv(uint entry, Packet* packet);
		void					Send(uint entry, PacketIpv4* packet);
	};
}
