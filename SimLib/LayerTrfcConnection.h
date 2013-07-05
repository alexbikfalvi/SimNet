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
#include "AddressIpv4.h"
#include "PacketTrfcFeedback.h"
#include "PacketTrfcMessage.h"
#include "PacketTrfcData.h"
#include "SimTimer.h"
#include "LayerTrfcTag.h"
#include "Delegate.h"
#include "Event.h"

namespace SimLib
{
	class LayerTrfcConnection : public Layer
	{
	public:
		enum EConnectionType
		{
			REQUESTER = 0,
			RESPONDER = 1
		};

		enum EState
		{
			CLOSED = 0,
			OPENING = 1,
			OPENED = 2,
			CLOSING = 3,
			CANCELING = 4,
			WAITING = 5
		};

		enum EOpenResult
		{
			OPEN_SUCCESS = 0,
			OPEN_CANCELED = 1,
			OPEN_FAIL_TIMEOUT = 2,
			OPEN_FAIL_REMOTE = 3
		};

		enum ECloseResult
		{
			CLOSE_CONFIRMED = 0,
			CLOSE_TIMEOUT = 1,
			CLOSE_COMPLETE = 2
		};

		typedef Delegate5<LayerTrfcConnection, void, __uint16, __uint16, AddressIpv4, __byte, Packet*>	TDelegateSend;
		typedef IDelegate5<void, __uint16, __uint16, AddressIpv4, __byte, Packet*>						TIDelegateSend;
		typedef IDelegate1<void, LayerTrfcConnection*>													TIDelegateDispose;

		typedef Event2<void, LayerTrfcConnection*, EOpenResult>											TEventOpen;
		typedef Event2<void, LayerTrfcConnection*, ECloseResult>										TEventClose;

	private:
		static uint						idGlobal;
		static uint						flowGlobal;

		// Timer timeout values
		static __time					timerOpenTimeout;
		static __time					timerCloseTimeout;
		static __time					timerCancelTimeout;
		static __time					timerRequesterWaitTimeout;
		static __time					timerResponderWaitTimeout;
		static __time					timerDefaultWaitTimeout;

		// Functions
		void							(LayerTrfcConnection::*functionOpen)();
		void							(LayerTrfcConnection::*functionClose)();
	
		void							(LayerTrfcConnection::*functionRecvMessageOpen)(PacketTrfcMessage*);
		void							(LayerTrfcConnection::*functionRecvMessageOpenAck)(PacketTrfcMessage*);
		void							(LayerTrfcConnection::*functionRecvMessageClose)(PacketTrfcMessage*);
		void							(LayerTrfcConnection::*functionRecvMessageCloseAck)(PacketTrfcMessage*);
		void							(LayerTrfcConnection::*functionRecvMessageCloseAckAck)(PacketTrfcMessage*);

		// Tag
		LayerTrfcTag*					tag;

	protected:
		// Type
		EConnectionType					type;

		// State
		EState							state;

		// Flow
		uint							flow;

		// Local connection
		uint							id;
		__uint16						port;

		// Remote connection
		AddressIpv4						remoteAddress;
		uint							remoteId;
		__uint16						remotePort;

		// Timers
		SimTimer<LayerTrfcConnection>*	timerControl;

		// Delegates
		TIDelegateSend&					delegateSend;
		TIDelegateDispose&				delegateDispose;

		// Events
		TEventOpen*						eventOpen;
		TEventClose*					eventClose;

		// String
		static char*					strConnectionType[];
		static char*					strState[];
		char							str[256];

	public:
		LayerTrfcConnection(
			EConnectionType		type,
			__uint16			port,
			SimHandler&			sim,
			TIDelegateSend&		delegateSend,
			TIDelegateDispose&	delegateDispose,
			AddressIpv4			remoteAddress,
			__uint16			remotePort
			);
		virtual ~LayerTrfcConnection();

		inline EConnectionType	Type() { return this->type; }
		inline EState			State() { return this->state; }

		inline uint				Id() { return this->id; }
		inline __uint16			Port() { return this->port; }

		inline uint				RemoteId() { return this->remoteId; }
		inline AddressIpv4		RemoteAddress() { return this->remoteAddress; }
		inline __uint16			RemotePort() { return this->remotePort; }

		// Events
		inline TEventOpen*		EventOpen() { return this->eventOpen; }
		inline TEventClose*		EventClose() { return this->eventClose; }

		// Functions
		void					Open();
		void					Close();

		// Receive
		void					Recv(AddressIpv4 src, PacketTrfc* packet);

		// Tag
		inline LayerTrfcTag*	Tag() { return this->tag; }
		inline void				Tag(LayerTrfcTag* tag) { this->tag = tag; }

		char*					ToString() const;

		virtual void			Initialize();
		virtual void			Finalize();

	protected:
		// Send
		bool					SendData(PacketTrfcData* packet);

		// Receive
		virtual void			Recv(PacketTrfc* packet) = 0;
		virtual void			RecvData(PacketTrfcData* packet) = 0;
		virtual void			RecvFeedback(PacketTrfcFeedback* packet) = 0;
		void					RecvMessage(PacketTrfcMessage* packet);

		void					RecvMessageOpenRequester(PacketTrfcMessage* packet);
		void					RecvMessageOpenAckRequester(PacketTrfcMessage* packet);
		void					RecvMessageCloseRequester(PacketTrfcMessage* packet);
		void					RecvMessageCloseAckRequester(PacketTrfcMessage* packet);
		void					RecvMessageCloseAckAckRequester(PacketTrfcMessage* packet);

		void					RecvMessageOpenResponder(PacketTrfcMessage* packet);
		void					RecvMessageOpenAckResponder(PacketTrfcMessage* packet);
		void					RecvMessageCloseResponder(PacketTrfcMessage* packet);
		void					RecvMessageCloseAckResponder(PacketTrfcMessage* packet);
		void					RecvMessageCloseAckAckResponder(PacketTrfcMessage* packet);

		// Functions
		void					OpenRequester();
		void					OpenResponder();

		void					CloseRequester();
		void					CloseResponder();

		// Timers
		void					TimerOpen(SimTimerInfo* info);	
		void					TimerClose(SimTimerInfo* info);	
		void					TimerCancel(SimTimerInfo* info);
		void					TimerWait(SimTimerInfo* info);
	};
}