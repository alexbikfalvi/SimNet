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
#include "LayerIpMcastGroup.h"
#include "LayerIpMcastMembership.h"
#include "PacketIpv4.h"
#include "Delegate.h"
#include "Event.h"

namespace SimLib
{
	class LayerIpMcast : public Layer
	{
	public:
		typedef Delegate2<LayerIpMcast, void, uint, PacketIpv4*>						TDelegateRecv;
		typedef Delegate1<LayerIpMcast, void, PacketIpv4*>								TDelegateSend;

		typedef Delegate3<LayerIpMcast, void, uint, AddressIpv4, LayerIpMcastGroup*>	TDelegateOp;
		typedef Delegate2<LayerIpMcast, void, uint, AddressIpv4>						TDelegateLocalOp;

		typedef IDelegate2<void, uint, PacketIpv4*>										TIDelegateRecv;
		typedef IDelegate1<void, PacketIpv4*>											TIDelegateSend;

		typedef IDelegate3<void, uint, AddressIpv4, LayerIpMcastGroup*>					TIDelegateOp;
		typedef IDelegate2<void, uint, AddressIpv4>										TIDelegateLocalOp;

		typedef Event2<void, uint, PacketIpv4*>											TEventOp;

	private:
		std::map<uint, LayerIpMcastGroup>		groups;
		std::map<uint, LayerIpMcastMembership>	memberships;

		// Delegates
		TDelegateRecv*							delegateRecv;
		TDelegateSend*							delegateSend;

		TDelegateOp*							delegateJoin;
		TDelegateOp*							delegateLeave;

		TDelegateLocalOp*						delegateLocalJoin;
		TDelegateLocalOp*						delegateLocalLeave;

		// Events
		TEventOp*								eventRecv;
		TEventOp*								eventSend;

	public:
		LayerIpMcast(
			SimHandler&	sim
			);
		virtual ~LayerIpMcast();

		inline TIDelegateRecv*		DelegateRecv() { return this->delegateRecv; }
		inline TIDelegateSend*		DelegateSend() { return this->delegateSend; }

		inline TIDelegateOp*		DelegateJoin() { return this->delegateJoin; }
		inline TIDelegateOp*		DelegateLeave() { return this->delegateLeave; }

		inline TIDelegateLocalOp*	DelegateLocalJoin() { return this->delegateLocalJoin; }
		inline TIDelegateLocalOp*	DelegateLocalLeave() { return this->delegateLocalLeave; }

		inline TEventOp*			EventRecv() { return this->eventRecv; }
		inline TEventOp*			EventSend() { return this->eventSend; }

		virtual void				Initialize();
		virtual void				Finalize() { }

	private:
		void						Recv(uint entry, PacketIpv4* packet);
		void						Send(PacketIpv4* packet);
	
		void						Join(uint entry, AddressIpv4 address, LayerIpMcastGroup* groupSender);
		void						Leave(uint entry, AddressIpv4 address, LayerIpMcastGroup* groupSender);

		void						LocalJoin(uint entry, AddressIpv4 address);
		void						LocalLeave(uint entry, AddressIpv4 address);
	};
}
