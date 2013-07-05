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
#include "PacketIgmp.h"
#include "Delegate.h"
#include "Event.h"

namespace SimLib
{
	class LayerIgmp : public Layer
	{
	public:
		enum EIgmpType
		{
			IGMP_HOST = 0,
			IGMP_ROUTER = 1
		};

		typedef Delegate2<LayerIgmp, void, uint, PacketIpv4*>		TDelegateRecv;

		typedef Delegate2<LayerIgmp, void, AddressIpv4, uint>		TDelegateJoin;
		typedef Delegate1<LayerIgmp, void, AddressIpv4>					TDelegateLeave;

		typedef IDelegate2<void, uint, PacketIpv4*>					TIDelegateRecv;

		typedef IDelegate2<void, AddressIpv4, uint>					TIDelegateJoin;
		typedef IDelegate1<void, AddressIpv4>							TIDelegateLeave;

		typedef Event3<void, uint, AddressIpv4, LayerIpMcastGroup*>	TEventJoin;
		typedef Event3<void, uint, AddressIpv4, LayerIpMcastGroup*>	TEventLeave;

		typedef Event2<void, uint, AddressIpv4>						TEventLocalJoin;
		typedef Event2<void, uint, AddressIpv4>						TEventLocalLeave;

		typedef Event2<void, uint, PacketIpv4*>						TEventSend;

		typedef std::map<AddressIpv4, LayerIpMcastMembership>			TMemberships;
		typedef std::map<AddressIpv4, LayerIpMcastGroup>				TGroups;

	
	private:
		AddressIpv4					address;
		EIgmpType					type;
	
		TMemberships				memberships;	// Group memberships initiated locally
		TGroups						groups;			// Group memberships initiated remotely

		TDelegateRecv*				delegateRecv;

		TDelegateJoin*				delegateJoin;
		TDelegateLeave*				delegateLeave;

		TEventJoin*					eventJoin;
		TEventLeave*				eventLeave;

		TEventLocalJoin*			eventLocalJoin;
		TEventLocalLeave*			eventLocalLeave;

		TEventSend*					eventSend;

		double						statEntries;
		uint					statEntriesNum;
		__time						statEntriesLast;

	public:
		LayerIgmp(
			SimHandler&	sim,
			AddressIpv4	address,
			EIgmpType	type = IGMP_HOST
			);
		virtual ~LayerIgmp();

		inline TIDelegateRecv*		DelegateRecv() { return this->delegateRecv; }
		inline TIDelegateJoin*		DelegateJoin() { return this->delegateJoin; }
		inline TIDelegateLeave*		DelegateLeave() { return this->delegateLeave; }

		inline TEventSend*			EventSend() { return this->eventSend; }
		inline TEventJoin*			EventJoin() { return this->eventJoin; }
		inline TEventLeave*			EventLeave() { return this->eventLeave; }
		
		inline TEventLocalJoin*		EventLocalJoin() { return this->eventLocalJoin; }
		inline TEventLocalLeave*	EventLocalLeave() { return this->eventLocalLeave; }

		inline double				StatEntries() { return this->statEntries; }

		virtual void				Initialize();
		virtual void				Finalize();

		void						Join(AddressIpv4 address, uint entry);
		void						Leave(AddressIpv4 address);

	private:
		void						Recv(uint entry, PacketIpv4* packet);
		void						RecvIgmpAllSystems(uint entry, AddressIpv4 src, AddressIpv4 dst, PacketIgmp* packet);
		void						RecvIgmpAllRouters(uint entry, AddressIpv4 src, AddressIpv4 dst, PacketIgmp* packet);
		void						RecvIgmpOther(uint entry, AddressIpv4 src, AddressIpv4 dst, PacketIgmp* packet);
		void						RecvIgmpJoinGroup(uint entry, AddressIpv4 src, AddressIpv4 dst, PacketIgmp* packet);
		void						RecvIgmpLeaveGroup(uint entry, AddressIpv4 src, AddressIpv4 dst, PacketIgmp* packet);
	
		void						Send(uint entry, AddressIpv4 dst, __byte ttl, Packet* payload);
	};
}