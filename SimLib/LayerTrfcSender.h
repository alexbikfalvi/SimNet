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

#include "LayerTrfcConnection.h"

#define RECV_RATE_SET_SIZE	4

namespace SimLib
{
	class LayerTrfcSender : public LayerTrfcConnection
	{
	public:
		enum EResult
		{
			SEND_SUCCESS = 0,
			SEND_FAIL_NOT_OPEN = 1
		};

		enum EStateSender
		{
			IDLE = 0,
			SENDING = 1
		};

		typedef std::queue<Packet*>						TBufferQueue;
		typedef std::map<uint, Packet*>					TBufferMap;

		typedef Event2<void, LayerTrfcSender*, Packet*>	TEventSend;

	private:

		// State
		EStateSender				stateSender;

		// Buffer
		TBufferQueue				bufferTx;					// transmission buffer (packets to be tranmitted)
		TBufferMap					bufferAck;					// acknowledgment buffer (packets transmitted but now acknowledged, either received or lost)
		TBufferQueue				bufferRtx;					// retransmission buffer (packets acknowledged as lost)

		// Flow
		__time						flowRtt;					// round-trip time
		__time						flowRto;					// retransmission time-out
		__time						flowTld;					// list-time doubled

		uint						flowMss;					// flow maximum segment size in bits
		__bitrate					flowRate;					// flow rate (bps)
		__bitrate					flowRateBps;				// flow rate calculated with TCP throughput equation
		__bitrate					flowInitialRate;			// flow initial rate (bps)

		double						flowLossRate;				// flow last reported loss rate (p)

		uint						flowPacketSequence;
		uint						flowPacketId;
		uint						flowPacketSize;

		static double				flowThroughputFunction[];	// flow throughput function f(p)
		static __time				flowMaximumBackoffInterval;

		bool						flowIdleNoFeedback;

		// Data-limited interval
		__time						dliNotLimited1;
		__time						dliNotLimited2;
		__time						dliTimeNew;
		__time						dliTimeNext;

		// Receiver
		__bitrate					recvRateSet[RECV_RATE_SET_SIZE];
		__byte						recvRateSetFirst;
		__byte						recvRateSetLast;

		// Timers
		SimTimer<LayerTrfcSender>*	timerNoFeedback;
		SimTimer<LayerTrfcSender>*	timerSender;

		// Functions
		void						(LayerTrfcSender::*functionSender)();

		// Events
		TEventSend*					eventSend;

	public:
		LayerTrfcSender(
			__uint16			port,
			SimHandler&			sim,
			TIDelegateSend&		delegateSend,
			TIDelegateDispose&	delegateDispose,
			AddressIpv4			remoteAddress,
			__uint16			remotePort,
			uint				remoteId,
			uint				segmentSize
			);
		virtual ~LayerTrfcSender();

		// Send
		EResult				Send(Packet* packet);

		// Event send
		inline TEventSend*	EventSend() { return this->eventSend; }

		// Stat
		inline __time		StatFlowRtt() { return this->flowRtt; }
		inline __bitrate	StatFlowRate() { return this->flowRate; }
		inline double		StatFlowLossRate() { return this->flowLossRate; }

		char*				ToString() const;
	private:
		// Receive
		virtual void		Recv(PacketTrfc* packet);
		virtual void		RecvData(PacketTrfcData* packet) { /* do nothing */ }
		virtual void		RecvFeedback(PacketTrfcFeedback* packet);

		// Timers
		void				TimerNoFeedback(SimTimerInfo* info);
		void				TimerSender(SimTimerInfo* info);

		void				TimerSenderIdle();
		void				TimerSenderSending();

		// Congestion control
		void				UpdateLimits(__bitrate limit);
	};
}
