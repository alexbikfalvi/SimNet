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

#define MAX_LOSS_EVENTS	8

namespace SimLib
{
	class LayerTrfcReceiver : public LayerTrfcConnection
	{
	public:
		struct SLossEvent
		{
			__time	firstLossTime;
			uint	firstLossSequence;
		};

		/*
		 * For performance reasons, the loss packets history is implemented as a FIFO queue and the receiver does not check if an out-of-order
		 * packet has been previously added (it is not possible to have out-of-order packets). Otherwise, it is possible that a receiver might
		 * report the same packet as both acknowledged and loss at the same time. If the implementation will support out of order packets,
		 * such a check mush be made either at the receiver or at the sender (better).
		 */
		typedef std::map<uint, __time>							THistoryRecv;
		typedef std::queue<uint>								THistoryLoss;

		typedef IDelegate2<void, LayerTrfcReceiver*, Packet*>	TIDelegateRecv;

	private:
		// Delegate
		TIDelegateRecv&					delegateRecv;

		// Flow
		double							flowLossRate;
		__bitrate						flowRateRecv;
		__time							flowRtt;
		uint							flowSequenceLast;		// last received sequence number
		__time							flowTimeLast;			// last received time
	
		__time							timeTxLast;				// transmit time of last received data packet
		__time							timeRxLast;				// receive time of last received data packet
		__time							timeFeedbackLast;		// last expiration of the feedback timer
	
		// Timers
		SimTimer<LayerTrfcReceiver>*	timerFeedback;

		// Receive function
		void							(LayerTrfcReceiver::*recvData)(PacketTrfcData*);

		// History
		THistoryRecv					historyRecv;
		THistoryLoss					historyLoss;

		// Loss
		SLossEvent						lossEvents[MAX_LOSS_EVENTS];
		__byte							lossEventFirst;
		__byte							lossEventLast;
		__byte							lossEventCount;
		static double					lossIntervalWeigth[MAX_LOSS_EVENTS];
		uint							lossSequenceLast;
		double							lossPrevInterval0;
		double							lossPrevInterval1;
		double							lossPrevWeigth;

		uint							recvSize;

	public:
		LayerTrfcReceiver(
			__uint16			port,
			SimHandler&			sim,
			TIDelegateSend&		delegateSend,
			TIDelegateRecv&		delegateRecv,
			TIDelegateDispose&	delegateDispose,
			AddressIpv4			remoteAddress,
			__uint16			remotePort
			);
		virtual ~LayerTrfcReceiver();

		char*			ToString() const;
	private:
		// Receive
		virtual void	Recv(PacketTrfc* packet);
		virtual void	RecvData(PacketTrfcData* packet);
		void			RecvDataFirst(PacketTrfcData* packet);
		void			RecvDataNext(PacketTrfcData* packet);
		virtual void	RecvFeedback(PacketTrfcFeedback* packet) { /* do nothing */ }

		// Timers
		void			TimerFeedback(SimTimerInfo* info);
	};
 }
