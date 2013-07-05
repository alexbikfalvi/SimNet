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

#include "Headers.h"
#include "LayerTrfcReceiver.h"
#include "PacketTrfcFeedback.h"

#pragma warning(disable : 4996)

#define	MIN(a,b)	((a<b)?a:b)
#define MAX(a,b)	((a>b)?a:b)

namespace SimLib
{
	double LayerTrfcReceiver::lossIntervalWeigth[MAX_LOSS_EVENTS] = {1.0, 1.0, 1.0, 1.0, 0.8, 0.6, 0.4, 0.2};

	LayerTrfcReceiver::LayerTrfcReceiver(
		__uint16			port,
		SimHandler&			sim,
		TIDelegateSend&		delegateSend,
		TIDelegateRecv&		delegateRecv,
		TIDelegateDispose&	delegateDispose,
		AddressIpv4			remoteAddress,
		__uint16			remotePort
		) : LayerTrfcConnection(REQUESTER, port, sim, delegateSend, delegateDispose, remoteAddress, remotePort), 
		delegateRecv(delegateRecv)
	{
		// Flow
		this->flowLossRate = 0;
		this->flowRateRecv = 0;
		this->flowSequenceLast = 0;
		this->flowTimeLast = 0;

		// Timers
		this->timerFeedback = alloc SimTimer<LayerTrfcReceiver>(this->sim, *this, &LayerTrfcReceiver::TimerFeedback);

		// Receive function : set to first
		this->recvData = &LayerTrfcReceiver::RecvDataFirst;

		// Loss
		this->lossEventFirst = 0;
		this->lossEventLast = 0;
		this->lossEventCount = 0;

		// Receive rate
		this->recvSize = 0;

		// Feedback
		this->timeFeedbackLast = 0;
	}

	LayerTrfcReceiver::~LayerTrfcReceiver()
	{
		// Timers
		delete this->timerFeedback;
	}

	void LayerTrfcReceiver::Recv(PacketTrfc *packet)
	{
		// General processing for all received packets (without considering the connection state)
	}

	void LayerTrfcReceiver::RecvData(PacketTrfcData* packet)
	{
		// Check the packet source and destination
		if(packet->Dst() != this->id) throw Exception(__FILE__, __LINE__, "LayerTrfcReceiver::RecvData - destination mismatch.");
		if(packet->Flow() != this->flow) throw Exception(__FILE__, __LINE__, "LayerTrfcReceiver::RecvData - flow mismatch.");

		// Check the connection state : must be OPENED or greater
		if(this->state < OPENED) return;

#ifdef LOG_FLOW
		Console::SetColor(Console::LIGHT_GREEN);
		printf("\n\tRECV T = %8.3lf RecvData() : %s %s @ %u", this->sim.Time(), this->ToString(), packet->Payload()->ToString(), packet->Sequence());
		Console::SetColor(Console::LIGHT_GRAY);
#endif

		// Global packet processing
		this->flowRtt = packet->Rtt();																// Round-trip-time

		if(this->flowRtt <= 0) throw Exception(__FILE__, __LINE__, "LayerTrfcReceiver::RecvData - flowRtt must be greater than zero (%lf).", this->flowRtt);

		// Process packet
		(this->*this->recvData)(packet);

		// Send payload to the delegate
		this->delegateRecv(this, packet->Payload());
	}

	void LayerTrfcReceiver::RecvDataFirst(PacketTrfcData* packet)
	{
		// Send a feedback packet
		PacketTrfcFeedback* reply = alloc PacketTrfcFeedback(
			this->flow,
			this->id,
			this->remoteId,
			this->sim.Time(),
			packet->TimeTx(),
			0,					// Time delay
			this->flowRateRecv,
			this->flowLossRate,
			1,					// Bitmap of acknowledged packets ( **** 0001)
			packet->Sequence(),	// SEQ of first acknowledged packet
			0,					// Bitmap of loss packets (N/A)
			0					// SEQ of first loss packet
			);

		this->delegateSend(this->port, this->remotePort, this->remoteAddress, 128, reply);

#ifdef LOG_FLOW
		Console::SetColor(Console::LIGHT_RED);
		printf("\n\tRECV T = %8.3lf SendFeedback() : %s", this->sim.Time(), this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
		printf("\n\t\tACK : %u", packet->Sequence());
#endif

		// Do not add the received packet info to packet history - the packet has already been acknowledged

		// Set the maximum sequence number and time to this packet (as first packet)
		this->flowSequenceLast = packet->Sequence();
		this->flowTimeLast = this->sim.Time();

		// Add the payload size to the received size
		this->recvSize += (packet->Payload())?packet->Payload()->Size():0;

		// Set the feedback timer
		this->timerFeedback->SetAfter(this->flowRtt);

		// Receive function : set to next
		this->recvData = &LayerTrfcReceiver::RecvDataNext;
	}

	void LayerTrfcReceiver::RecvDataNext(PacketTrfcData* packet)
	{
		/*
		 * Step 0
		 */
		// Record the Tx and Rx time for last received packet
		this->timeRxLast = packet->TimeTx();
		this->timeTxLast = this->sim.Time();

		// Add the payload size to the received size
		this->recvSize += (packet->Payload())?packet->Payload()->Size():0;

		/*
		 * Step 1 : Add the received packet info to acknowledgement history : packet SEQ and received time
		 */
		this->historyRecv.insert(std::pair<uint, __time>(packet->Sequence(), this->sim.Time()));

		/*
		 * Step 2 : Check if packet indicates a alloc loss event
		 */

		bool newLossEvent = false;

		// Calculate whether there are missing packets between this packet and the maximum ID number
		for(uint seq = this->flowSequenceLast + 1; seq < packet->Sequence(); seq++)
		{
			// The packets with this SEQ is missing : add the missing SEQ at the end of the loss history
			this->historyLoss.push(seq);

			// Calculate the interpolated time of this lost packet
			__time lossTime = this->flowTimeLast + (this->sim.Time() - this->flowTimeLast) * (seq - this->flowSequenceLast) / (packet->Sequence() - this->flowSequenceLast);

			// If there exists a last loss event
			if(this->lossEventCount > 0)
			{
				// Check if the packet belongs to the last loss event
			
				// If the loss time is not within an RTT from the last loss event
				if(this->lossEvents[this->lossEventLast].firstLossTime + this->flowRtt < lossTime)
				{
					// Add lost packet belongs to a alloc loss event

					// Increment the last loss event index
					this->lossEventLast = (++this->lossEventLast) % MAX_LOSS_EVENTS;

					// If the maximum number of loss events has been reached
					if(this->lossEventFirst == this->lossEventLast)
					{
						// Overwrite the first loss event
						if(MAX_LOSS_EVENTS != this->lossEventCount) throw Exception(__FILE__, __LINE__, "LayerTrfcReceiver::RecvDataNext - lossEventCount (%u) must equal MAX_LOSS_EVENTS (%u).", this->lossEventCount, MAX_LOSS_EVENTS);
						this->lossEventFirst = (++this->lossEventFirst) % MAX_LOSS_EVENTS;
					}
					else
						// Increment the loss event count
						this->lossEventCount++;

					// Add the time and sequence number to the last loss event
					this->lossEvents[this->lossEventLast].firstLossSequence = seq;	// The sequence of the lost packet
					this->lossEvents[this->lossEventLast].firstLossTime = lossTime;	// The interpolated loss time

					// Set there has been a alloc loss event
					newLossEvent = true;
				}
				// Else the lost packet belongs to the last loss event
			}
			else
			{
				// Create the first loss event
				if(this->lossEventFirst != this->lossEventLast) throw Exception(__FILE__, __LINE__, "lossEventFirst (%u) and lossEventLast (%u) must be equal.", this->lossEventFirst, this->lossEventLast);
			
				// Add the time and sequence number to the last loss event
				this->lossEvents[this->lossEventLast].firstLossSequence = seq;
				this->lossEvents[this->lossEventLast].firstLossTime = lossTime;

				// Increment the loss event count
				this->lossEventCount++;

				// Set there has been a alloc loss event
				newLossEvent = true;
			}
		}

		// Set the maximum sequence number and time to this packet (as last packet)
		this->flowSequenceLast = packet->Sequence();
		this->flowTimeLast = this->sim.Time();

		/*
		 * Step 3 : Calculate loss rate (p)
		 */

		// If there has been a alloc loss event
		if(newLossEvent)
		{
			if(0 == this->lossEventCount) throw Exception(__FILE__, __LINE__, "LayerTrfcReceiver::RecvDataNext - lossEventCount must be greater than zero.");

			// Set the last packet for which the loss rate is calculated
			this->lossSequenceLast = packet->Sequence();

			double lossInterval;

			// Reset previous lost intervals
			this->lossPrevInterval0 = 0;
			this->lossPrevInterval1 = 0;
			this->lossPrevWeigth = 0;

			// Calculate the previous loss intervals is the diff beween the the loss events
			for(__byte index = 1; index < this->lossEventCount; index++)
			{
				__byte begin = ((this->lossEventLast + MAX_LOSS_EVENTS) - index) % MAX_LOSS_EVENTS;
				__byte end = ((this->lossEventLast + MAX_LOSS_EVENTS) - index + 1) % MAX_LOSS_EVENTS;

				lossInterval = this->lossEvents[end].firstLossSequence - this->lossEvents[begin].firstLossSequence;

				this->lossPrevInterval0 += lossInterval * this->lossIntervalWeigth[index];
				this->lossPrevInterval1 += lossInterval * this->lossIntervalWeigth[index-1];
				this->lossPrevWeigth += this->lossIntervalWeigth[index];
			}

			// Calculate the current loss interva
			double lossInterval0 = (this->flowSequenceLast - this->lossEvents[this->lossEventLast].firstLossSequence + 1) * this->lossIntervalWeigth[0] + this->lossPrevInterval0;
			double lossWeight = this->lossIntervalWeigth[0] + this->lossPrevWeigth;
			lossInterval = MAX(lossInterval0, this->lossPrevInterval1);

			// Calculate the average loss interval
			double rateLoss = lossWeight / lossInterval;

			// If current loss rate is greater than previous loss rate (early loss detection)
			if(rateLoss > this->flowLossRate)
			{
				// Set the loss rate
				this->flowLossRate = rateLoss;

				// Expire the feedback timer
				if(!this->timerFeedback->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcReceiver::RecvDataNext - the feedback timer must be set.");
				this->timerFeedback->Cancel();

				// Send feedback
				this->TimerFeedback(NULL);
			}
			else
			{
				// Set the loss rate
				this->flowLossRate = rateLoss;
			}
		}
		// Else, do nothing


		// Check the feedback timer is set
		if(!this->timerFeedback->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcReceiver::RecvDataNext - the feedback timer must be set.");
	}

	void LayerTrfcReceiver::TimerFeedback(SimTimerInfo* info)
	{
		// The feedback timer expired

		// Only if at least one packet has been received since the last feedback

		if(!this->historyRecv.empty())
		{
#ifdef LOG_FLOW
			Console::SetColor(Console::LIGHT_RED);
			printf("\n\tRECV T = %8.3lf TimerFeedback() : %s (RTT=%.3lf)", this->sim.Time(), this->ToString(), this->flowRtt);
			Console::SetColor(Console::LIGHT_GRAY);
#endif
			/*
			 * Step 1 : Calculate the loss rate
			 */
		
			// If the last sequence number is different from the one for which the loss was previously calculated; 
			// and there are loss events
			if((this->flowSequenceLast != this->lossSequenceLast) && (this->lossEventCount))
			{
				// Use the partial data calculated at the last loss event

				double lossInterval0 = (this->flowSequenceLast - this->lossEvents[this->lossEventLast].firstLossSequence + 1) * this->lossIntervalWeigth[0];
				double lossWeigth = this->lossIntervalWeigth[0] + this->lossPrevWeigth;
				double lossInterval = MAX(lossInterval0, this->lossPrevInterval1);

				this->flowLossRate = lossWeigth / lossInterval;
			}

			/*
			 * Step 2 : Calculate the received rate since the last expiration of the feedback timer
			 */
			this->flowRateRecv = this->recvSize / (this->sim.Time() - this->timeFeedbackLast);

			// Reset the received size
			this->recvSize = 0;

			/*
			 * Step 3 : Calculate the packets to acknowledge or report as loss
			 */
		
			// Acknowledge ALL packets in the received history
			uint ackSeq = 0;
			__uint64 ack = 0;

			if(this->historyRecv.size() > 0)	// If there are packets to acknowledge
			{
#ifdef LOG_FLOW
				printf("\n\t\tACK : ");
#endif
				// Select the first packet
				THistoryRecv::iterator iter = this->historyRecv.begin();
				ackSeq = iter->first;

				// Select the next packets
				for(; iter != this->historyRecv.end(); iter++)
				{
					// Calculate the bitmap index as the difference between the current ID and first ID
					__byte bit = iter->first - ackSeq;

					// Current implementation only alows acknowledgment of a 64 range of packets at a time
					if(bit >= 64) throw Exception(__FILE__, __LINE__, "LayerTrfcReceiver::TimerFeedback - maximum acknowledgment range is 63 (%u).", bit);

					// Set the corresponing bit to 1
					ack |= (((__uint64)1) << bit);

#ifdef LOG_FLOW
					printf("%u ", iter->first);
#endif
				}

				// Clear the received history : all packets have been acknowledged
				this->historyRecv.clear();
			}

			// Report packets in the loss history with
			uint lossSeq = 0;
			__uint64 loss = 0;

			if(this->historyLoss.size() > 0)	// If there are packets loss
			{
#ifdef LOG_FLOW
				printf("\n\t\tLOST : ");
#endif

				// Select the first packet
				lossSeq = this->historyLoss.front();

				// Select the next packets
				while(!this->historyLoss.empty())
				{
					// Get the front SEQ
					uint seq = this->historyLoss.front();

					// Remove the front SEQ
					this->historyLoss.pop();

					// Calculate the bitmap index as the difference between the current ID and the first ID
					__byte bit = seq - lossSeq;
				
					// Current implementation only alows loss reporting of a 64 range of packets at a time
					if(bit >= 64) throw Exception(__FILE__, __LINE__, "LayerTrfcReceiver::TimerFeedback - maximum acknowledgement range is 63 (%u).", bit);

					// Set the corresponding bit to 1
					loss |= (((__uint64)1) << bit);

#ifdef LOG_FLOW
					printf("%u ", seq);
#endif
				}
			}

			// Send a feedback packet
			PacketTrfcFeedback* reply = alloc PacketTrfcFeedback(
				this->flow,
				this->id,
				this->remoteId,
				this->timeTxLast,
				this->timeRxLast,
				this->sim.Time() - this->timeTxLast, // Time delay : current time minus last Tx time
				this->flowRateRecv,
				this->flowLossRate,
				ack,				// Bitmap of acknowledged packets ( **** 0001)
				ackSeq,				// SEQ of first acknowledged packet
				loss,				// Bitmap of loss packets (N/A)
				lossSeq				// SEQ of first loss packet
				);
			this->delegateSend(this->port, this->remotePort, this->remoteAddress, 128, reply);
		}
		else
		{
			// Do nothing
#ifdef LOG_FLOW
			Console::SetColor(Console::LIGHT_YELLOW);
			printf("\n\tRECV T = %8.3lf TimerFeedback() : %s (RTT=%.3lf)", this->sim.Time(), this->ToString(), this->flowRtt);
			Console::SetColor(Console::LIGHT_GRAY);
#endif
		}
	
		// Set the current time of the expiration of the feedback timer
		this->timeFeedbackLast = this->sim.Time();

		// Set the feedback timer after one RTT
		this->timerFeedback->SetAfter(this->flowRtt);
	}


	char* LayerTrfcReceiver::ToString() const
	{
		sprintf((char*)this->str, "[ l=*:%u:%u r=%u:%u:%u t=%s s=%s recv=%u loss=%u ]",
			this->port, this->id, ((AddressIpv4)this->remoteAddress).Addr(), this->remotePort, this->remoteId,
			LayerTrfcConnection::strConnectionType[this->type], LayerTrfcConnection::strState[this->state],
			this->historyRecv.size(), this->historyLoss.size()
			);

		return (char*)this->str;
	}
}
