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
#include "LayerPimSmMembership.h"
#include "LayerIpMcastGroup.h"
#include "Route.h"
#include "PacketIpv4.h"
#include "PacketPimSm.h"
#include "PacketPimSmJoinPrune.h"
#include "Delegate.h"
#include "Event.h"

namespace SimLib
{
	class LayerPimSm : public Layer
	{
	public:
		typedef Delegate2<LayerPimSm, void, uint, PacketIpv4*>						TDelegateRecv;
		typedef Delegate3<LayerPimSm, void, uint, AddressIpv4, LayerIpMcastGroup*>	TDelegateOp;

		typedef IDelegate2<void, uint, PacketIpv4*>									TIDelegateRecv;
		typedef IDelegate3<void, uint, AddressIpv4, LayerIpMcastGroup*>				TIDelegateOp;

		typedef Event3<void, uint, AddressIpv4, LayerIpMcastGroup*>					TEventOp;
		typedef Event2<void, uint, PacketIpv4*>										TEventSend;

	private:
		AddressIpv4								address;
		std::vector<AddressIpv4>&				rp;
		Route&									route;

		std::map<uint, LayerPimSmMembership>	memberships;
		std::map<uint, LayerIpMcastGroup>		groups;

		TDelegateRecv*							delegateRecv;

		TDelegateOp*							delegateJoin;
		TDelegateOp*							delegateLeave;

		TEventOp*								eventJoin;
		TEventOp*								eventLeave;

		TEventSend*								eventSend;

		double									statEntries;
		uint									statEntriesNum;
		__time									statEntriesLast;

	public:
		LayerPimSm(
			SimHandler&					sim,
			AddressIpv4					address,
			std::vector<AddressIpv4>&	rp,
			Route&						route
			);
		virtual ~LayerPimSm();

		inline TIDelegateRecv*		DelegateRecv() { return this->delegateRecv;}

		inline TIDelegateOp*		DelegateJoin() { return this->delegateJoin; }
		inline TIDelegateOp*		DelegateLeave() { return this->delegateLeave; }

		inline TEventOp*			EventJoin() { return this->eventJoin; }
		inline TEventOp*			EventLeave() { return this->eventLeave; }

		inline TEventSend*			EventSend() { return this->eventSend; }

		inline double				StatEntries() { return this->statEntries; }

		virtual void				Initialize();
		virtual void				Finalize();

	private:
		void						Recv(uint entry, PacketIpv4* packet);
		void						RecvPimSm(uint entry, AddressIpv4 src, AddressIpv4 dst, PacketPimSm* packet);
		void						RecvPimSmJoinPrune(uint entry, AddressIpv4 src, AddressIpv4 dst, PacketPimSmJoinPrune* packet);

		void						Send(uint entry, AddressIpv4 dst, __byte ttl, Packet* payload);

		void						Join(uint entry, AddressIpv4 address, LayerIpMcastGroup* igmp);
		void						Leave(uint entry, AddressIpv4 address, LayerIpMcastGroup* igmp);

		void						JoinGroup(uint group, AddressIpv4 address);
		void						LeaveGroup(uint group, AddressIpv4 address);
	};
}
