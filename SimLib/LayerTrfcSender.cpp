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
#include "LayerTrfcSender.h"

#pragma warning(disable : 4996)

#define MIN(a,b)	((a<b)?a:b)
#define	MAX(a,b)	((a>b)?a:b)

namespace SimLib
{
	__time LayerTrfcSender::flowMaximumBackoffInterval = 64.0;

	LayerTrfcSender::LayerTrfcSender(
		__uint16			port,
		SimHandler&			sim,
		TIDelegateSend&		delegateSend,
		TIDelegateDispose&	delegateDispose,
		AddressIpv4			remoteAddress,
		__uint16			remotePort,
		uint				remoteId,
		uint				segmentSize
		) : LayerTrfcConnection(RESPONDER, port, sim, delegateSend, delegateDispose, remoteAddress, remotePort)
	{
		// Remote
		this->remoteId = remoteId;

		// State
		this->stateSender = IDLE;

		// Flow (initial values)
		this->flowRtt = -1;
		this->flowRto = 2;
		this->flowTld = -1;

		this->flowMss = segmentSize;
		this->flowRate = 0;
		this->flowRateBps = 0;

		this->flowLossRate = 0;
		this->flowPacketSequence = 0;
		this->flowPacketId = 0;
		this->flowPacketSize = segmentSize;

		// Data-limited interval
		this->dliNotLimited1 = 0;
		this->dliNotLimited2 = 0;
		this->dliTimeNew = 0;
		this->dliTimeNext = 0;

		// Receiver
		this->recvRateSetFirst = 0;
		this->recvRateSetLast = 0;

		// Timers
		this->timerNoFeedback = alloc SimTimer<LayerTrfcSender>(this->sim, *this, &LayerTrfcSender::TimerNoFeedback);
		this->timerSender = alloc SimTimer<LayerTrfcSender>(this->sim, *this, &LayerTrfcSender::TimerSender);

		// Functions
		this->functionSender = &LayerTrfcSender::TimerSenderIdle;
	
		// Events
		this->eventSend = alloc Event2<void, LayerTrfcSender*, Packet*>();
	}

	LayerTrfcSender::~LayerTrfcSender()
	{
		// Delete packets remaining in the buffers
			// Transmission buffer
		while(!this->bufferTx.empty())
		{
			this->bufferTx.front()->Delete();
			delete this->bufferTx.front();
			this->bufferTx.pop();
		}
			// Retransmission buffer
		while(!this->bufferRtx.empty())
		{
			this->bufferRtx.front()->Delete();
			delete this->bufferRtx.front();
			this->bufferRtx.pop();
		}
			// Acknowledgment buffer
		for(TBufferMap::iterator iter = this->bufferAck.begin(); iter != this->bufferAck.end(); iter++)
		{
			iter->second->Delete();
			delete iter->second;
		}

		// Timers
		delete this->timerNoFeedback;
		delete this->timerSender;

		// Events
		delete this->eventSend;
	}

	LayerTrfcSender::EResult LayerTrfcSender::Send(Packet* packet)
	{
		// Check the connection type
		if(RESPONDER != this->type) throw Exception(__FILE__, __LINE__, "LayerTrfcSender::Send - connection type is not RESPONDER (%u).", this->type);

		// Check the connection is opened
		if(OPENED != this->state)
		{
			// Delete the packet
			packet->Delete();
			delete packet;

			return SEND_FAIL_NOT_OPEN;
		}

		// Add the packet to the transmission buffer and increment the packet ID (packet IDs are assigned only here;
		// any retransmission of the packet will carry the same ID)
		this->bufferTx.push(packet);

		// Check the state of the sender
		switch(this->stateSender)
		{
		case IDLE:
			// If the sender is in IDLE state, send the frame directly

			// Check the feedback and sender timers are not set
			if(this->timerSender->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcSender::Send - sender timer must not be set.");

			// Call the sender timer handler
			this->TimerSender(NULL);

			break;
		case SENDING:
			// If the sender is in SENDING or WAITING states, do nothing
			break;
		}

		// Update the flow packet information
		this->flowMss = MAX(this->flowMss, packet->Size());									// Maximum packet size
		this->flowPacketSize = (uint)(0.1 * this->flowPacketSize + 0.9 * packet->Size());	// Average packet size

		return SEND_SUCCESS;
	}

	void LayerTrfcSender::Recv(PacketTrfc *packet)
	{
		// General processing for all received packets (without considering the connection state)

		// Calculate RTT according to RFC 5348
		// If packet does not contain RTT information (RX timestamp is less than zero), skip packet
		if(packet->TimeRx() < 0) return;

		if(this->flowRtt < 0)
		{
			// If this is the first packet with RTT information
			this->flowRtt = this->sim.Time() - packet->TimeRx() - packet->TimeDelay();
			if(this->flowRtt <= 0) throw Exception(__FILE__, __LINE__, "LayerTrfcSender::Recv - flowRtt must be greater than zero (%lf).", this->flowRtt);

			// Set TLD timestamp
			this->flowTld = this->sim.Time();

			// Calculate initial flow rate
			this->flowInitialRate =  MIN(4 * this->flowMss, MAX(2 * this->flowMss, 35040)) / this->flowRtt;
			this->flowRate = this->flowInitialRate;
		}
	}

	void LayerTrfcSender::RecvFeedback(PacketTrfcFeedback* packet)
	{
		// Feedback processing is done after general processing

		// Check the packet source and destination
		if(packet->Dst() != this->id) throw Exception(__FILE__, __LINE__, "LayerTrfcSender::RecvFeedback - destination mismatch.");
		if(packet->Flow() != this->flow) throw Exception(__FILE__, __LINE__, "LayerTrfcSender::RecvFeedback - flow mismatch.");

		// Check the connection state : must be OPENED or greater
		if(this->state < OPENED) return;

		// If the feedback timer is set, stop the feedback timer
		if(this->timerNoFeedback->IsSet()) this->timerNoFeedback->Cancel();

		/*
		 * Step 1: Update the RTT and RTO
		 */

		this->flowRtt = 0.9 * this->flowRtt + 0.1 * (this->sim.Time() - packet->TimeRx() - packet->TimeDelay());
		if(this->flowRtt <= 0) throw Exception(__FILE__, __LINE__, "LayerTrfcSender::RecvFeedback - flowRtt must be greater than zero (%lf).", this->flowRtt);

		this->flowRto = MAX(4 * this->flowRtt, 2.0 * this->flowPacketSize / this->flowRate);
		if(this->flowRto <= 0) throw Exception(__FILE__, __LINE__, "LayerTrfcSender::RecvFeedback - flowRto must be greater than zero (%lf).", this->flowRto);

		/*
		 * Step 2 : Update the allowed sending rate
		 */

		__bitrate recvLimit;

		// Determine if the current feedback interval was data-limited
		bool dli;
		{
			this->dliTimeNew = packet->TimeRx();			// The transmission time of the last received data packet
			__time dliTimeOld = this->dliTimeNew - this->flowRtt;	// Estimated time for the beginning of the interval
			this->dliTimeNext = this->sim.Time();

			dli = !(((dliTimeOld < this->dliNotLimited1) && (this->dliNotLimited1 <= this->dliTimeNew)) ||
				((dliTimeOld < this->dliNotLimited2) && (this->dliNotLimited2 <= this->dliTimeNew)));

			if((this->dliNotLimited1 <=this->dliTimeNew) && (this->dliNotLimited2 > this->dliTimeNew))
				this->dliNotLimited1 = this->dliNotLimited2;
		}

		if(dli)
		{
			// If the interval covered by the feedback packet was data limited
			if(packet->RateLoss() > this->flowLossRate)
			{
				// If the feedback packet reports an increase in the loss rate

				// Halve all entries in the receiver rate set
				__bitrate recvRateMaxValue = -1;
				__byte recvRateMaxIndex;
				for(__byte index = this->recvRateSetFirst; (index % RECV_RATE_SET_SIZE) != this->recvRateSetLast;
					index = (++index) % RECV_RATE_SET_SIZE)
				{
					this->recvRateSet[index] /= 2;
				
					if(recvRateMaxValue < this->recvRateSet[index])
					{
						recvRateMaxValue = this->recvRateSet[index];
						recvRateMaxIndex = index;
					}
				}

				__bitrate recvRate = 0.85 * packet->RateRecv();

				// Maximize the receiver rate set
				if(recvRate > recvRateMaxValue)
				{
					// Set the alloc value as the maximum and only value
					this->recvRateSet[0] = recvRate;
					this->recvRateSetFirst = 0;
					this->recvRateSetLast = 1;
					recvLimit = recvRate;
				}
				else
				{
					// Keep the maximum as the only value and update its timestamp
					this->recvRateSet[recvRateMaxIndex] = this->sim.Time();
					this->recvRateSetFirst = recvRateMaxIndex;
					this->recvRateSetLast = (recvRateMaxIndex + 1) % RECV_RATE_SET_SIZE;
					recvLimit = recvRateMaxValue;
				}
			}
			else
			{
				// Maximize the receiver set
				__bitrate recvRateMaxValue = -1;
				__byte recvRateMaxIndex;
			
				for(__byte index = this->recvRateSetFirst; (index % RECV_RATE_SET_SIZE) != this->recvRateSetLast;
					index = (++index) % RECV_RATE_SET_SIZE)
				{
					if(recvRateMaxValue < this->recvRateSet[index])
					{
						recvRateMaxValue = this->recvRateSet[index];
						recvRateMaxIndex = index;
					}
				}

				if(packet->RateRecv() > recvRateMaxValue)
				{
					// Set the alloc value as the maximum and only value
					this->recvRateSet[0] = packet->RateRecv();
					this->recvRateSetFirst = 0;
					this->recvRateSetLast = 1;
					recvLimit = 2 * packet->RateRecv();
				}
				else
				{
					// Keep the maximum as the only value
					this->recvRateSetFirst = recvRateMaxIndex;
					this->recvRateSetLast = (recvRateMaxIndex + 1) % RECV_RATE_SET_SIZE;
					recvLimit = 2 * recvRateMaxValue;
				}
			}
		}
		else
		{
			// Update the receiver set
			this->recvRateSet[this->recvRateSetLast] = packet->RateRecv();

			this->recvRateSetLast = (++this->recvRateSetLast) % RECV_RATE_SET_SIZE;
			if(this->recvRateSetFirst == this->recvRateSetLast) this->recvRateSetFirst = (++this->recvRateSetFirst) % RECV_RATE_SET_SIZE;

			// Calculate the maximum
			__bitrate recvRateMaxValue = -1;
			for(__byte index = this->recvRateSetFirst; (index % RECV_RATE_SET_SIZE) != this->recvRateSetLast;
				index = (++index) % RECV_RATE_SET_SIZE)
			{
				if(recvRateMaxValue < this->recvRateSet[index])
					recvRateMaxValue = this->recvRateSet[index];
			}

			if(recvRateMaxValue < 0) throw Exception(__FILE__, __LINE__, "LayerTrfcSender::RecvFeedback - recvRateMaxValue must be greater than or equal to zero (%lf).", recvRateMaxValue);

			recvLimit = 2 * recvRateMaxValue;
		}

#ifndef NO_CONGESTION_CONTROL
		if(packet->RateLoss() > 0)
		{
			// Congestion avoidance
			uint rateLossIndex = (uint)(10000 * packet->RateLoss());
			if(rateLossIndex > 10000) rateLossIndex = 10000;
		
			this->flowRateBps = this->flowPacketSize / (this->flowRtt * LayerTrfcSender::flowThroughputFunction[rateLossIndex]);

			// Calculate the flow rate
			this->flowRate = MAX(MIN(this->flowRateBps, recvLimit), this->flowPacketSize / LayerTrfcSender::flowMaximumBackoffInterval);
		}
		else if(this->sim.Time() - this->flowTld >= this->flowRtt)
		{
			// Initial slow-start

			this->flowRate = MAX(MIN(2 * this->flowRate, recvLimit), this->flowInitialRate);
			this->flowTld = this->sim.Time();
		}
#endif

		/*
		 * Step 3 : Save received information from the last feedback packet
		 */
		this->flowLossRate = packet->RateLoss();

#ifdef LOG_FLOW
		Console::SetColor(Console::LIGHT_RED);
		printf("\n\tSEND T = %8.3lf RecvFeedback() : %s : \n\t\tRTO=%lf s=%u X=%lf p=%lf X_recv=%lf dli=%u recv_limit=%lf",
			this->sim->Time(), this->ToString(), this->flowRto, this->flowPacketSize, this->flowRate,
			packet->RateLoss(), packet->RateRecv(), dli, recvLimit);
		Console::SetColor(Console::LIGHT_GRAY);
#endif

		/*
		 * Step 4 : Process acknowledged and lost packets
		 */

		// Process the acknowledged packets
		if(packet->Ack())
		{
#ifdef LOG_FLOW
			printf("\n\t\tACK : ");
#endif
			uint seq = packet->AckSeq();
			for(__uint64 ack = packet->Ack(); ack; ack = ack >> 1)
			{
				// For every acknowledgement bit
				if(ack & 1)
				{
#ifdef LOG_FLOW
					printf("%u ", seq);
#endif
					// If the packet is marked as acknowledged, remove the packet from the acknowledgement buffer
					TBufferMap::iterator iter = this->bufferAck.find(seq);
					if(iter == this->bufferAck.end()) throw Exception(__FILE__, __LINE__, "LayerTrfcSender::RecvFeedback - packet %u not found in the acknowledgment buffer.", seq);

					// Delete the packet
					iter->second->Delete();
					delete iter->second;

					// Erase the iterator
					this->bufferAck.erase(iter);
				}
				// Increment the packet ID
				seq++;
			}
		}

		// Process the lost packets
		if(packet->Lost())
		{
#ifdef LOG_FLOW
			printf("\n\t\tLOST : ");
#endif
			uint seq = packet->LostSeq();
			for(__uint64 lost = packet->Lost(); lost; lost = lost >> 1)
			{
				// For every lost bit
				if(lost & 1)
				{
#ifdef LOG_FLOW
					printf("%u ", seq);
#endif
					// If the packet is marked as lost, remove the packet from the acknowledgement buffer
					TBufferMap::iterator iter = this->bufferAck.find(seq);
					if(iter == this->bufferAck.end()) throw Exception(__FILE__, __LINE__, "LayerTrfcSender::RecvFeedback - packet %u not found in the acknowledgment buffer.", seq);

					// Add the packet to the retransmission buffer
					this->bufferRtx.push(iter->second);

					// Erase the iterator
					this->bufferAck.erase(iter);
				}
				// Increment the ID
				seq++;
			}
		}

		// If the feedback reported a loss and the sender is in IDLE
		if((IDLE == this->stateSender) && (!this->bufferRtx.empty()))
		{
			// Start sending

			// Check the sender timer is not set
			if(this->timerSender->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcSender::RecvFeedback - the sender timer must not be set.");

			// Change the sender state
			this->stateSender = SENDING;

			// Set the sender as no longer idle
			this->flowIdleNoFeedback = false;

			// Change the timer function
			this->functionSender = &LayerTrfcSender::TimerSenderSending;

			// Call the sending function
			this->TimerSenderSending();
		}
	}

	void LayerTrfcSender::TimerNoFeedback(SimTimerInfo* info)
	{
		// If the connection is not OPENED or greater, cancel sending
		if(this->state < OPENED) return;

#ifdef LOG_FLOW
		Console::SetColor(Console::LIGHT_MAGENTA);
		printf("\n\tSEND T = %8.3lf TimerNoFeedback() : %s", this->sim->Time(), this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif

	#ifndef NO_CONGESTION_CONTROL

		// Calculate the maximum received rate
		__bitrate recvRate = -1;
		for(__byte index = this->recvRateSetFirst; (index % RECV_RATE_SET_SIZE) != this->recvRateSetLast;
			index = (++index) % RECV_RATE_SET_SIZE)
		{
			if(recvRate < this->recvRateSet[index])
				recvRate = this->recvRateSet[index];
		}

		// If the maximum received rate is negative, set to infinity
		if(recvRate < 0) recvRate = 1E308;

		if((((this->flowLossRate > 0) && (recvRate < this->flowInitialRate)) ||
			((this->flowLossRate == 0) && (this->flowRate < 2 * this->flowInitialRate))) &&
			this->flowIdleNoFeedback)
		{
			// Do nothing
		}
		else if(this->flowLossRate == 0)
		{
			// Halve the allowed sending rate
			this->flowRate = MAX(this->flowRate / 2, this->flowPacketSize / LayerTrfcSender::flowMaximumBackoffInterval);
		}
		else if(this->flowRateBps > 2 * recvRate)
		{
			// Twice the receiver rate is already limiting the sending rate : halve the sending rate
			this->UpdateLimits(recvRate);
		}
		else
		{
			// The sending rate is limited by X_bps : halve the sending rate
			this->UpdateLimits(this->flowRateBps / 2);
		}

	#endif

		// Reset the no-feedback timer
		this->timerNoFeedback->SetAfter(MAX(4*this->flowRtt, 2*this->flowPacketSize/this->flowRate));
	}

	void LayerTrfcSender::TimerSender(SimTimerInfo* info)
	{
		// If the connection is not OPENED, cancel sending
		if(this->state < OPENED) return;

		// Call the sender function
		(this->*this->functionSender)();
	}

	void LayerTrfcSender::TimerSenderIdle()
	{
		// Check the sender state
		if(IDLE != this->stateSender) throw Exception(__FILE__, __LINE__, "LayerTrfcSender::TimerSenderIdle - the sender state must be IDLE (%u).", this->stateSender);

		// Check the retransmission buffer is empty (otherwise the sender cannot be in IDLE state)
		if(!this->bufferRtx.empty()) throw Exception(__FILE__, __LINE__, "LayerTrfcSender::TimerSenderIdle - the retransmission buffer must be empty.");
		// Check the transmission buffer is not empty (otherwise the timer could not be set)
		if(this->bufferTx.empty()) throw Exception(__FILE__, __LINE__, "LayerTrfcSender::TimerSenderIdle - the transmission buffer must not be empty.");

		// Change the sender state
		this->stateSender = SENDING;

		// Set the sender as no longer idle
		this->flowIdleNoFeedback = false;

		// Change the timer function
		this->functionSender = &LayerTrfcSender::TimerSenderSending;

		// Call the sending function
		this->TimerSenderSending();
	}

	void LayerTrfcSender::TimerSenderSending()
	{
		// Check the sender state
		if(SENDING != this->stateSender) throw Exception(__FILE__, __LINE__, "LayerTrfcSender::TimerSenderSending - the sender state must be SENDING (%u).", this->stateSender);

		// Check that at least one of the transmission or retransmission buffer are not empty
		if(this->bufferTx.empty() && this->bufferRtx.empty()) throw Exception(__FILE__, __LINE__, "LayerTrfcSender::TimerSenderSending - at least one of the transmission or retransmission buffer must not be empty.");

		// If the retransmission timer is not empty, there are packets to be retransmitted : use the retransmission buffer
		// Else, use the transmission buffer
		TBufferQueue* buffer = (!this->bufferRtx.empty()) ? &this->bufferRtx : &this->bufferTx;

		// Get the first packet from the selected buffer
		Packet* payload = buffer->front();
		if(NULL == payload) throw Exception(__FILE__, __LINE__, "LayerTrfcSender::TimerSenderSending - the buffer packet must not be null.");

		// Remove the first packet from the selected buffer
		buffer->pop();

		// Create a alloc data packet with a copy of the payload and a alloc sequence number
		PacketTrfcData* packet = alloc PacketTrfcData(
			this->flow,
			this->id,
			this->remoteId,
			payload->Copy(),	// add a copy of the payload
			this->flowPacketSequence,
			this->sim.Time(),
			this->flowRtt
			);

		// Send the data packet
		this->delegateSend(this->port, this->remotePort, this->remoteAddress, 128, packet);

		// Raise the send event
		(*this->eventSend)(this, packet);

		// Add the packet to the acknowledgment buffer, mapped to the packet ID
		this->bufferAck.insert(std::pair<uint, Packet*>(this->flowPacketSequence, payload));

#ifdef LOG_FLOW
		Console::SetColor(Console::LIGHT_CYAN);
		printf("\n\tSEND T = %8.3lf TimerSenderSending() : %s : %s @ %u", this->sim->Time(), this->ToString(), payload->ToString(), this->flowPacketSequence);
		Console::SetColor(Console::LIGHT_GRAY);
#endif

		// Increment the packet sequence number
		this->flowPacketSequence++;

		// If the any of the transmission or retransmission buffers are not empty
		if((!this->bufferTx.empty()) || (!this->bufferRtx.empty()))
		{
			// Reschedule the sending timer after the transmission duration alotted to the current packet (packet size / transmission rate)
			this->timerSender->SetAfter(payload->Size() / this->flowRate);
		}
		else
		{
			// Do not reschedule the timer and change the state to idle
			this->functionSender = &LayerTrfcSender::TimerSenderIdle;

			this->stateSender = IDLE;
		}

		// If the no-feedback timer is not set, set the no-feedback timer
		if(!this->timerNoFeedback->IsSet())
		{
#ifdef LOG_FLOW
			Console::SetColor(Console::LIGHT_YELLOW);
			printf("\t : Feedback timer set +%.3lf (RTT=%.3lf)", this->flowRto, this->flowRtt);
			Console::SetColor(Console::LIGHT_GRAY);
#endif
		
			this->timerNoFeedback->SetAfter(this->flowRto);

			// Mark the sender if idle
			this->flowIdleNoFeedback = (IDLE == this->stateSender);
		}
		else
		{
#ifdef LOG_FLOW
			Console::SetColor(Console::LIGHT_CYAN);
			printf("\t : Feedback timer in-progress (RTT=%.3lf)", this->flowRtt);
			Console::SetColor(Console::LIGHT_GRAY);
#endif
		}

		// Update the state for the data-limited interval
		if(this->dliNotLimited1 <= this->dliTimeNew)
			this->dliNotLimited1 = this->sim.Time();
		else if(this->dliNotLimited2 <= this->dliTimeNext)
			this->dliNotLimited2 = this->sim.Time();
	}

	void LayerTrfcSender::UpdateLimits(__bitrate limit)
	{
		if(limit < this->flowPacketSize / LayerTrfcSender::flowMaximumBackoffInterval)
			limit = this->flowPacketSize / LayerTrfcSender::flowMaximumBackoffInterval;

		// Replace receiver set with a single item : limit / 2
		this->recvRateSet[0] = limit / 2;
		this->recvRateSetFirst = 0;
		this->recvRateSetLast = 1;

		// Recalculate the sending rate
		if(this->flowLossRate > 0)
		{
			// Congestion avoidance
			uint rateLossIndex = (uint)(10000 * this->flowLossRate);
			if(rateLossIndex > 10000) throw Exception(__FILE__, __LINE__, "LayerTrfcSender::UpdateLimits - rateLossIndex must not be greater than 10000 (%u).", rateLossIndex);
		
			this->flowRateBps = this->flowPacketSize / (this->flowRtt * LayerTrfcSender::flowThroughputFunction[rateLossIndex]);

			// Calculate the flow rate
			this->flowRate = MAX(MIN(this->flowRateBps, limit), this->flowPacketSize / LayerTrfcSender::flowMaximumBackoffInterval);
		}
		else if(this->sim.Time() - this->flowTld >= this->flowRtt)
		{
			// Initial slow-start

			this->flowRate = MAX(MIN(2 * this->flowRate, limit), this->flowInitialRate);
			this->flowTld = this->sim.Time();
		}	
	}

	char* LayerTrfcSender::ToString() const
	{
		sprintf((char*)this->str, "[ l=*:%u:%u r=%u:%u:%u t=%s s=%s tx=%u ack=%u rtx=%u ]",
			this->port, this->id, ((AddressIpv4)this->remoteAddress).Addr(), this->remotePort, this->remoteId,
			LayerTrfcConnection::strConnectionType[this->type], LayerTrfcConnection::strState[this->state],
			this->bufferTx.size(), this->bufferAck.size(), this->bufferRtx.size()
			);

		return (char*)this->str;
	}

	double LayerTrfcSender::flowThroughputFunction[] = {
		0.000000000,0.008172314,0.011567790,0.014180319,0.016388720,0.018339578,0.020108001,0.021738567,0.023260291,0.024693311,0.026052275,0.027348232,0.028589755,0.029783660,0.030935465,0.032049715,0.033130204,0.034180139,0.035202259,0.036198925,
		0.037172188,0.038123847,0.039055484,0.039968504,0.040864159,0.041743571,0.042607750,0.043457609,0.044293976,0.045117604,0.045929184,0.046729347,0.047518672,0.048297697,0.049066913,0.049826780,0.050577722,0.051320135,0.052054387,0.052780822,
		0.053499762,0.054211508,0.054916343,0.055614534,0.056306331,0.056991970,0.057671673,0.058345652,0.059014105,0.059677222,0.060335182,0.060988153,0.061636299,0.062279773,0.062918721,0.063553282,0.064183590,0.064809772,0.065431949,0.066050237,
		0.066664747,0.067275586,0.067882856,0.068486654,0.069087074,0.069684207,0.070278138,0.070868952,0.071456728,0.072041544,0.072623473,0.073202587,0.073778955,0.074352645,0.074923719,0.075492240,0.076058268,0.076621861,0.077183075,0.077741964,
		0.078298580,0.078852974,0.079405195,0.079955291,0.080503308,0.081049290,0.081593282,0.082135325,0.082675459,0.083213726,0.083750163,0.084284807,0.084817696,0.085348864,0.085878346,0.086406175,0.086932384,0.087457005,0.087980069,0.088501605,
		0.089021642,0.089540211,0.090057337,0.090573049,0.091087373,0.091600334,0.092111958,0.092622269,0.093131292,0.093639049,0.094145564,0.094650858,0.095154955,0.095657874,0.096159637,0.096660264,0.097159776,0.097658191,0.098155528,0.098651807,
		0.099147045,0.099641261,0.100134471,0.100626693,0.101117944,0.101608240,0.102097597,0.102586031,0.103073556,0.103560190,0.104045945,0.104530838,0.105014881,0.105498089,0.105980476,0.106462054,0.106942838,0.107422841,0.107902074,0.108380550,
		0.108858282,0.109335282,0.109811561,0.110287131,0.110762003,0.111236189,0.111709700,0.112182546,0.112654737,0.113126285,0.113597200,0.114067491,0.114537169,0.115006243,0.115474724,0.115942619,0.116409939,0.116876693,0.117342889,0.117808537,
		0.118273645,0.118738222,0.119202276,0.119665816,0.120128849,0.120591384,0.121053428,0.121514990,0.121976077,0.122436696,0.122896856,0.123356562,0.123815824,0.124274647,0.124733039,0.125191006,0.125648556,0.126105696,0.126562431,0.127018769,
		0.127474715,0.127930277,0.128385460,0.128840271,0.129294716,0.129748800,0.130202530,0.130655912,0.131108951,0.131561654,0.132014025,0.132466070,0.132917796,0.133369206,0.133820307,0.134271104,0.134721602,0.135171806,0.135621722,0.136071353,
		0.136520707,0.136969786,0.137418596,0.137867143,0.138315430,0.138763462,0.139211244,0.139658780,0.140106075,0.140553134,0.140999961,0.141446560,0.141892935,0.142339091,0.142785032,0.143230762,0.143676285,0.144121606,0.144566728,0.145011655,
		0.145456392,0.145900941,0.146345308,0.146789496,0.147233508,0.147677348,0.148121021,0.148564529,0.149007877,0.149451067,0.149894104,0.150336991,0.150779732,0.151222329,0.151664787,0.152107108,0.152549297,0.152991355,0.153433288,0.153875097,
		0.154316786,0.154758358,0.155199817,0.155641166,0.156082407,0.156523543,0.156964579,0.157405516,0.157846358,0.158287107,0.158727767,0.159168341,0.159608831,0.160049240,0.160489572,0.160929828,0.161370012,0.161810126,0.162250173,0.162690156,
		0.163130078,0.163569941,0.164009747,0.164449500,0.164889202,0.165328856,0.165768464,0.166208028,0.166647552,0.167087037,0.167526486,0.167965902,0.168405287,0.168844644,0.169283974,0.169723281,0.170162566,0.170601832,0.171041081,0.171480316,
		0.171919538,0.172358751,0.172797956,0.173237156,0.173676352,0.174115548,0.174554745,0.174993945,0.175433150,0.175872364,0.176311587,0.176750822,0.177190071,0.177629336,0.178068619,0.178507923,0.178947249,0.179386599,0.179825975,0.180265380,
		0.180704815,0.181144282,0.181583784,0.182023321,0.182462897,0.182902513,0.183342171,0.183781873,0.184221620,0.184661415,0.185101260,0.185541156,0.185981105,0.186421109,0.186861170,0.187301290,0.187741470,0.188181712,0.188622018,0.189062390,
		0.189502830,0.189943339,0.190383919,0.190824572,0.191265299,0.191706102,0.192146983,0.192587944,0.193028986,0.193470111,0.193911320,0.194352616,0.194794000,0.195235473,0.195677037,0.196118694,0.196560446,0.197002293,0.197444238,0.197886282,
		0.198328427,0.198770674,0.199213025,0.199655481,0.200098044,0.200540716,0.200983498,0.201426391,0.201869398,0.202312519,0.202755756,0.203199111,0.203642585,0.204086179,0.204529895,0.204973735,0.205417700,0.205861791,0.206306010,0.206750359,
		0.207194838,0.207639449,0.208084194,0.208529074,0.208974091,0.209419245,0.209864538,0.210309972,0.210755548,0.211201268,0.211647132,0.212093143,0.212539301,0.212985607,0.213432064,0.213878673,0.214325435,0.214772351,0.215219422,0.215666650,
		0.216114037,0.216561583,0.217009290,0.217457160,0.217905193,0.218353390,0.218801754,0.219250285,0.219698985,0.220147855,0.220596896,0.221046110,0.221495497,0.221945059,0.222394798,0.222844714,0.223294809,0.223745085,0.224195541,0.224646180,
		0.225097003,0.225548011,0.225999205,0.226450587,0.226902157,0.227353917,0.227805868,0.228258012,0.228710349,0.229162881,0.229615609,0.230068534,0.230521657,0.230974980,0.231428503,0.231882228,0.232336157,0.232790289,0.233244627,0.233699172,
		0.234153924,0.234608885,0.235064056,0.235519438,0.235975033,0.236430841,0.236886864,0.237343103,0.237799558,0.238256232,0.238713125,0.239170238,0.239627572,0.240085130,0.240542910,0.241000916,0.241459148,0.241917606,0.242376293,0.242835209,
		0.243294356,0.243753734,0.244213344,0.244673188,0.245133267,0.245593582,0.246054134,0.246514924,0.246975953,0.247437222,0.247898733,0.248360486,0.248822482,0.249284723,0.249747209,0.250209943,0.250672924,0.251136154,0.251599633,0.252063364,
		0.252527347,0.252991582,0.253456072,0.253920817,0.254385819,0.254851077,0.255316594,0.255782370,0.256248407,0.256714705,0.257181266,0.257648090,0.258115179,0.258582533,0.259050154,0.259518043,0.259986200,0.260454627,0.260923325,0.261392295,
		0.261861537,0.262331054,0.262800845,0.263270912,0.263741256,0.264211878,0.264682779,0.265153959,0.265625421,0.266097165,0.266569191,0.267041502,0.267514097,0.267986979,0.268460147,0.268933604,0.269407349,0.269881384,0.270355711,0.270830329,
		0.271305240,0.271780445,0.272255945,0.272731741,0.273207834,0.273684225,0.274160915,0.274637905,0.275115195,0.275592787,0.276070682,0.276548881,0.277027385,0.277506194,0.277985311,0.278464734,0.278944467,0.279424509,0.279904862,0.280385526,
		0.280866503,0.281347794,0.281829399,0.282311319,0.282793556,0.283276110,0.283758983,0.284242175,0.284725687,0.285209520,0.285693676,0.286178155,0.286662958,0.287148086,0.287633540,0.288119321,0.288605430,0.289091867,0.289578635,0.290065734,
		0.290553164,0.291040927,0.291529023,0.292017455,0.292506221,0.292995324,0.293484765,0.293974544,0.294464662,0.294955121,0.295445921,0.295937063,0.296428548,0.296920377,0.297412551,0.297905071,0.298397938,0.298891152,0.299384715,0.299878628,
		0.300372892,0.300867507,0.301362475,0.301857796,0.302353471,0.302849502,0.303345888,0.303842632,0.304339734,0.304837195,0.305335016,0.305833198,0.306331742,0.306830648,0.307329918,0.307829552,0.308329552,0.308829919,0.309330652,0.309831754,
		0.310333225,0.310835067,0.311337279,0.311839863,0.312342821,0.312846152,0.313349857,0.313853939,0.314358397,0.314863232,0.315368446,0.315874039,0.316380013,0.316886367,0.317393104,0.317900224,0.318407728,0.318915616,0.319423890,0.319932551,
		0.320441600,0.320951037,0.321460864,0.321971080,0.322481689,0.322992689,0.323504082,0.324015870,0.324528052,0.325040630,0.325553605,0.326066977,0.326580748,0.327094919,0.327609490,0.328124462,0.328639836,0.329155614,0.329671795,0.330188382,
		0.330705374,0.331222773,0.331740580,0.332258796,0.332777421,0.333296456,0.333815902,0.334335761,0.334856033,0.335376719,0.335897820,0.336419337,0.336941270,0.337463622,0.337986391,0.338509581,0.339033190,0.339557221,0.340081674,0.340606551,
		0.341131851,0.341657576,0.342183727,0.342710305,0.343237310,0.343764744,0.344292608,0.344820901,0.345349626,0.345878783,0.346408374,0.346938398,0.347468857,0.347999752,0.348531083,0.349062852,0.349595060,0.350127707,0.350660794,0.351194323,
		0.351728294,0.352262707,0.352797565,0.353332867,0.353868616,0.354404810,0.354941453,0.355478543,0.356016084,0.356554074,0.357092515,0.357631409,0.358170756,0.358710556,0.359250811,0.359791522,0.360332689,0.360874314,0.361416398,0.361958940,
		0.362501943,0.363045407,0.363589332,0.364133721,0.364678574,0.365223891,0.365769674,0.366315923,0.366862640,0.367409825,0.367957479,0.368505603,0.369054199,0.369603266,0.370152806,0.370702820,0.371253309,0.371804273,0.372355714,0.372907631,
		0.373460028,0.374012903,0.374566258,0.375120095,0.375674413,0.376229214,0.376784499,0.377340268,0.377896523,0.378453264,0.379010492,0.379568209,0.380126415,0.380685110,0.381244297,0.381803976,0.382364147,0.382924812,0.383485971,0.384047626,
		0.384609777,0.385172426,0.385735572,0.386299218,0.386863364,0.387428010,0.387993159,0.388558810,0.389124964,0.389691623,0.390258788,0.390826459,0.391394637,0.391963323,0.392532518,0.393102224,0.393672440,0.394243167,0.394814408,0.395386162,
		0.395958430,0.396531214,0.397104515,0.397678332,0.398252668,0.398827522,0.399402897,0.399978792,0.400555209,0.401132149,0.401709612,0.402287600,0.402866113,0.403445153,0.404024719,0.404604814,0.405185438,0.405766592,0.406348277,0.406930493,
		0.407513242,0.408096525,0.408680342,0.409264695,0.409849584,0.410435010,0.411020974,0.411607478,0.412194521,0.412782105,0.413370231,0.413958900,0.414548113,0.415137870,0.415728172,0.416319021,0.416910417,0.417502361,0.418094855,0.418687898,
		0.419281492,0.419875639,0.420470338,0.421065590,0.421661398,0.422257760,0.422854680,0.423452156,0.424050191,0.424648785,0.425247939,0.425847655,0.426447932,0.427048772,0.427650176,0.428252144,0.428854678,0.429457779,0.430061447,0.430665684,
		0.431270490,0.431875866,0.432481813,0.433088332,0.433695424,0.434303091,0.434911332,0.435520148,0.436129542,0.436739513,0.437350062,0.437961191,0.438572900,0.439185191,0.439798064,0.440411520,0.441025560,0.441640184,0.442255395,0.442871193,
		0.443487578,0.444104552,0.444722116,0.445340270,0.445959016,0.446578354,0.447198285,0.447818811,0.448439932,0.449061649,0.449683963,0.450306874,0.450930385,0.451554495,0.452179207,0.452804520,0.453430435,0.454056954,0.454684077,0.455311806,
		0.455940141,0.456569084,0.457198634,0.457828794,0.458459564,0.459090944,0.459722937,0.460355542,0.460988761,0.461622595,0.462257045,0.462892111,0.463527794,0.464164096,0.464801018,0.465438559,0.466076722,0.466715507,0.467354915,0.467994948,
		0.468635605,0.469276888,0.469918798,0.470561336,0.471204502,0.471848298,0.472492725,0.473137784,0.473783474,0.474429799,0.475076757,0.475724351,0.476372581,0.477021449,0.477670954,0.478321099,0.478971883,0.479623309,0.480275376,0.480928087,
		0.481581441,0.482235440,0.482890084,0.483545375,0.484201314,0.484857901,0.485515137,0.486173024,0.486831563,0.487490753,0.488150597,0.488811095,0.489472248,0.490134057,0.490796523,0.491459647,0.492123430,0.492787873,0.493452977,0.494118742,
		0.494785170,0.495452262,0.496120019,0.496788441,0.497457529,0.498127285,0.498797709,0.499468803,0.500140567,0.500813002,0.501486110,0.502159890,0.502834345,0.503509475,0.504185281,0.504861764,0.505538925,0.506216764,0.506895284,0.507574484,
		0.508254366,0.508934931,0.509616180,0.510298113,0.510980732,0.511664037,0.512348030,0.513032711,0.513718082,0.514404143,0.515090895,0.515778340,0.516466478,0.517155310,0.517844838,0.518535061,0.519225982,0.519917600,0.520609918,0.521302936,
		0.521996655,0.522691075,0.523386199,0.524082026,0.524778558,0.525475796,0.526173741,0.526872393,0.527571754,0.528271825,0.528972607,0.529674099,0.530376305,0.531079224,0.531782857,0.532487206,0.533192272,0.533898054,0.534604555,0.535311776,
		0.536019716,0.536728378,0.537437762,0.538147870,0.538858701,0.539570258,0.540282541,0.540995551,0.541709289,0.542423756,0.543138953,0.543854881,0.544571541,0.545288934,0.546007061,0.546725923,0.547445521,0.548165855,0.548886928,0.549608739,
		0.550331290,0.551054581,0.551778615,0.552503391,0.553228911,0.553955175,0.554682185,0.555409942,0.556138447,0.556867700,0.557597702,0.558328455,0.559059960,0.559792217,0.560525228,0.561258993,0.561993514,0.562728791,0.563464825,0.564201618,
		0.564939171,0.565677484,0.566416558,0.567156394,0.567896994,0.568638358,0.569380488,0.570123384,0.570867047,0.571611478,0.572356679,0.573102650,0.573849393,0.574596907,0.575345195,0.576094257,0.576844094,0.577594708,0.578346098,0.579098267,
		0.579851215,0.580604943,0.581359453,0.582114744,0.582870819,0.583627678,0.584385322,0.585143752,0.585902970,0.586662975,0.587423770,0.588185355,0.588947731,0.589710899,0.590474860,0.591239615,0.592005166,0.592771512,0.593538656,0.594306598,
		0.595075339,0.595844880,0.596615222,0.597386366,0.598158314,0.598931065,0.599704622,0.600478985,0.601254155,0.602030133,0.602806920,0.603584518,0.604362927,0.605142147,0.605922181,0.606703029,0.607484693,0.608267172,0.609050469,0.609834584,
		0.610619518,0.611405272,0.612191848,0.612979246,0.613767467,0.614556512,0.615346383,0.616137080,0.616928604,0.617720956,0.618514138,0.619308150,0.620102994,0.620898670,0.621695179,0.622492522,0.623290701,0.624089717,0.624889570,0.625690261,
		0.626491792,0.627294163,0.628097376,0.628901431,0.629706330,0.630512073,0.631318662,0.632126097,0.632934380,0.633743512,0.634553493,0.635364325,0.636176009,0.636988545,0.637801935,0.638616180,0.639431280,0.640247238,0.641064053,0.641881727,
		0.642700261,0.643519655,0.644339912,0.645161032,0.645983015,0.646805864,0.647629578,0.648454160,0.649279609,0.650105928,0.650933117,0.651761177,0.652590109,0.653419915,0.654250594,0.655082149,0.655914580,0.656747889,0.657582076,0.658417142,
		0.659253088,0.660089917,0.660927627,0.661766221,0.662605700,0.663446064,0.664287315,0.665129453,0.665972481,0.666816398,0.667661205,0.668506905,0.669353497,0.670200983,0.671049364,0.671898641,0.672748815,0.673599887,0.674451858,0.675304730,
		0.676158502,0.677013176,0.677868754,0.678725236,0.679582623,0.680440917,0.681300118,0.682160227,0.683021246,0.683883175,0.684746016,0.685609770,0.686474437,0.687340019,0.688206517,0.689073931,0.689942263,0.690811514,0.691681685,0.692552777,
		0.693424791,0.694297728,0.695171589,0.696046376,0.696922088,0.697798728,0.698676296,0.699554794,0.700434221,0.701314581,0.702195873,0.703078098,0.703961258,0.704845354,0.705730386,0.706616356,0.707503265,0.708391114,0.709279904,0.710169636,
		0.711060311,0.711951929,0.712844494,0.713738004,0.714632462,0.715527868,0.716424223,0.717321529,0.718219786,0.719118996,0.720019159,0.720920278,0.721822352,0.722725382,0.723629371,0.724534319,0.725440226,0.726347095,0.727254926,0.728163719,
		0.729073478,0.729984201,0.730895891,0.731808548,0.732722174,0.733636769,0.734552335,0.735468873,0.736386384,0.737304868,0.738224327,0.739144762,0.740066175,0.740988565,0.741911934,0.742836284,0.743761615,0.744687929,0.745615226,0.746543507,
		0.747472774,0.748403028,0.749334270,0.750266500,0.751199720,0.752133931,0.753069135,0.754005331,0.754942521,0.755880707,0.756819889,0.757760069,0.758701247,0.759643425,0.760586603,0.761530783,0.762475966,0.763422152,0.764369344,0.765317542,
		0.766266747,0.767216960,0.768168182,0.769120415,0.770073659,0.771027915,0.771983186,0.772939470,0.773896771,0.774855089,0.775814424,0.776774779,0.777736153,0.778698549,0.779661967,0.780626409,0.781591875,0.782558366,0.783525884,0.784494430,
		0.785464004,0.786434609,0.787406244,0.788378912,0.789352613,0.790327347,0.791303118,0.792279924,0.793257768,0.794236651,0.795216574,0.796197537,0.797179542,0.798162590,0.799146682,0.800131820,0.801118003,0.802105234,0.803093514,0.804082842,
		0.805073222,0.806064653,0.807057137,0.808050675,0.809045269,0.810040918,0.811037624,0.812035389,0.813034213,0.814034098,0.815035044,0.816037053,0.817040126,0.818044263,0.819049467,0.820055738,0.821063077,0.822071485,0.823080964,0.824091514,
		0.825103137,0.826115834,0.827129606,0.828144453,0.829160378,0.830177380,0.831195462,0.832214625,0.833234869,0.834256195,0.835278605,0.836302100,0.837326681,0.838352348,0.839379104,0.840406950,0.841435885,0.842465912,0.843497032,0.844529245,
		0.845562553,0.846596957,0.847632458,0.848669057,0.849706755,0.850745554,0.851785454,0.852826456,0.853868563,0.854911774,0.855956091,0.857001515,0.858048048,0.859095689,0.860144441,0.861194305,0.862245281,0.863297371,0.864350575,0.865404896,
		0.866460334,0.867516890,0.868574566,0.869633361,0.870693279,0.871754319,0.872816483,0.873879772,0.874944187,0.876009729,0.877076400,0.878144200,0.879213131,0.880283193,0.881354388,0.882426717,0.883500181,0.884574782,0.885650519,0.886727395,
		0.887805411,0.888884567,0.889964866,0.891046307,0.892128892,0.893212623,0.894297500,0.895383524,0.896470697,0.897559020,0.898648493,0.899739119,0.900830898,0.901923831,0.903017919,0.904113164,0.905209567,0.906307128,0.907405850,0.908505732,
		0.909606777,0.910708985,0.911812358,0.912916896,0.914022601,0.915129474,0.916237515,0.917346727,0.918457111,0.919568667,0.920681396,0.921795300,0.922910380,0.924026637,0.925144072,0.926262687,0.927382482,0.928503458,0.929625618,0.930748961,
		0.931873489,0.932999203,0.934126104,0.935254194,0.936383474,0.937513944,0.938645606,0.939778462,0.940912511,0.942047756,0.943184197,0.944321835,0.945460673,0.946600710,0.947741949,0.948884390,0.950028034,0.951172882,0.952318936,0.953466197,
		0.954614666,0.955764344,0.956915233,0.958067332,0.959220645,0.960375171,0.961530912,0.962687868,0.963846043,0.965005435,0.966166047,0.967327880,0.968490934,0.969655211,0.970820713,0.971987439,0.973155393,0.974324573,0.975494983,0.976666622,
		0.977839493,0.979013595,0.980188931,0.981365502,0.982543308,0.983722352,0.984902633,0.986084153,0.987266914,0.988450916,0.989636161,0.990822650,0.992010384,0.993199364,0.994389591,0.995581067,0.996773792,0.997967769,0.999162997,1.000359478,
		1.001557214,1.002756205,1.003956453,1.005157959,1.006360724,1.007564749,1.008770035,1.009976584,1.011184396,1.012393474,1.013603817,1.014815427,1.016028306,1.017242455,1.018457874,1.019674565,1.020892529,1.022111767,1.023332281,1.024554071,
		1.025777139,1.027001486,1.028227113,1.029454021,1.030682212,1.031911686,1.033142446,1.034374491,1.035607823,1.036842444,1.038078354,1.039315555,1.040554048,1.041793833,1.043034913,1.044277289,1.045520961,1.046765931,1.048012200,1.049259768,
		1.050508639,1.051758811,1.053010288,1.054263069,1.055517157,1.056772551,1.058029254,1.059287267,1.060546590,1.061807226,1.063069175,1.064332437,1.065597016,1.066862911,1.068130124,1.069398657,1.070668509,1.071939683,1.073212180,1.074486001,
		1.075761146,1.077037618,1.078315418,1.079594546,1.080875003,1.082156792,1.083439913,1.084724368,1.086010156,1.087297281,1.088585743,1.089875543,1.091166682,1.092459162,1.093752984,1.095048148,1.096344657,1.097642511,1.098941712,1.100242260,
		1.101544157,1.102847405,1.104152003,1.105457955,1.106765260,1.108073920,1.109383936,1.110695309,1.112008041,1.113322133,1.114637586,1.115954400,1.117272578,1.118592121,1.119913030,1.121235305,1.122558948,1.123883961,1.125210345,1.126538100,
		1.127867228,1.129197731,1.130529609,1.131862863,1.133197495,1.134533506,1.135870898,1.137209671,1.138549826,1.139891365,1.141234290,1.142578601,1.143924299,1.145271385,1.146619862,1.147969730,1.149320990,1.150673644,1.152027692,1.153383137,
		1.154739978,1.156098218,1.157457858,1.158818898,1.160181340,1.161545186,1.162910436,1.164277092,1.165645154,1.167014625,1.168385505,1.169757796,1.171131498,1.172506613,1.173883143,1.175261088,1.176640449,1.178021229,1.179403427,1.180787046,
		1.182172086,1.183558550,1.184946436,1.186335749,1.187726487,1.189118653,1.190512248,1.191907274,1.193303730,1.194701619,1.196100942,1.197501699,1.198903893,1.200307524,1.201712594,1.203119104,1.204527055,1.205936448,1.207347285,1.208759566,
		1.210173294,1.211588469,1.213005092,1.214423165,1.215842689,1.217263665,1.218686094,1.220109978,1.221535318,1.222962115,1.224390370,1.225820085,1.227251261,1.228683898,1.230117999,1.231553564,1.232990596,1.234429094,1.235869060,1.237310495,
		1.238753402,1.240197780,1.241643631,1.243090957,1.244539758,1.245990036,1.247441792,1.248895028,1.250349744,1.251805942,1.253263623,1.254722788,1.256183439,1.257645576,1.259109202,1.260574316,1.262040921,1.263509018,1.264978608,1.266449692,
		1.267922272,1.269396348,1.270871922,1.272348996,1.273827569,1.275307645,1.276789223,1.278272306,1.279756894,1.281242988,1.282730591,1.284219703,1.285710325,1.287202459,1.288696105,1.290191266,1.291687942,1.293186135,1.294685846,1.296187076,
		1.297689826,1.299194098,1.300699893,1.302207212,1.303716057,1.305226428,1.306738327,1.308251755,1.309766714,1.311283204,1.312801227,1.314320785,1.315841877,1.317364507,1.318888674,1.320414381,1.321941628,1.323470417,1.325000749,1.326532625,
		1.328066047,1.329601015,1.331137532,1.332675597,1.334215214,1.335756382,1.337299103,1.338843378,1.340389209,1.341936597,1.343485543,1.345036048,1.346588114,1.348141742,1.349696933,1.351253688,1.352812009,1.354371897,1.355933353,1.357496379,
		1.359060976,1.360627144,1.362194886,1.363764202,1.365335094,1.366907563,1.368481611,1.370057238,1.371634446,1.373213236,1.374793610,1.376375568,1.377959112,1.379544243,1.381130963,1.382719273,1.384309174,1.385900667,1.387493754,1.389088435,
		1.390684713,1.392282588,1.393882062,1.395483136,1.397085811,1.398690088,1.400295969,1.401903456,1.403512548,1.405123248,1.406735558,1.408349477,1.409965008,1.411582151,1.413200909,1.414821281,1.416443271,1.418066878,1.419692104,1.421318951,
		1.422947419,1.424577511,1.426209226,1.427842567,1.429477535,1.431114131,1.432752356,1.434392212,1.436033700,1.437676821,1.439321577,1.440967968,1.442615997,1.444265664,1.445916970,1.447569917,1.449224507,1.450880740,1.452538618,1.454198142,
		1.455859313,1.457522133,1.459186603,1.460852725,1.462520498,1.464189926,1.465861008,1.467533747,1.469208144,1.470884200,1.472561915,1.474241293,1.475922333,1.477605038,1.479289407,1.480975444,1.482663149,1.484352522,1.486043567,1.487736283,
		1.489430673,1.491126737,1.492824477,1.494523894,1.496224989,1.497927764,1.499632220,1.501338359,1.503046180,1.504755687,1.506466880,1.508179760,1.509894329,1.511610589,1.513328539,1.515048182,1.516769520,1.518492552,1.520217281,1.521943708,
		1.523671834,1.525401660,1.527133189,1.528866420,1.530601356,1.532337997,1.534076346,1.535816403,1.537558169,1.539301646,1.541046836,1.542793739,1.544542357,1.546292690,1.548044742,1.549798512,1.551554002,1.553311214,1.555070148,1.556830806,
		1.558593190,1.560357300,1.562123138,1.563890706,1.565660003,1.567431033,1.569203796,1.570978293,1.572754526,1.574532497,1.576312205,1.578093654,1.579876843,1.581661775,1.583448450,1.585236870,1.587027037,1.588818951,1.590612615,1.592408028,
		1.594205193,1.596004111,1.597804783,1.599607211,1.601411395,1.603217338,1.605025040,1.606834503,1.608645728,1.610458716,1.612273469,1.614089988,1.615908275,1.617728330,1.619550155,1.621373752,1.623199122,1.625026265,1.626855184,1.628685879,
		1.630518352,1.632352605,1.634188638,1.636026453,1.637866052,1.639707435,1.641550604,1.643395560,1.645242305,1.647090839,1.648941165,1.650793284,1.652647196,1.654502904,1.656360408,1.658219710,1.660080811,1.661943713,1.663808416,1.665674923,
		1.667543234,1.669413351,1.671285276,1.673159008,1.675034551,1.676911905,1.678791071,1.680672052,1.682554847,1.684439459,1.686325889,1.688214138,1.690104207,1.691996099,1.693889813,1.695785352,1.697682717,1.699581909,1.701482930,1.703385781,
		1.705290462,1.707196977,1.709105325,1.711015508,1.712927528,1.714841386,1.716757084,1.718674621,1.720594001,1.722515224,1.724438292,1.726363205,1.728289966,1.730218576,1.732149035,1.734081346,1.736015509,1.737951527,1.739889400,1.741829129,
		1.743770717,1.745714164,1.747659472,1.749606642,1.751555675,1.753506573,1.755459337,1.757413969,1.759370469,1.761328840,1.763289082,1.765251197,1.767215187,1.769181052,1.771148793,1.773118413,1.775089913,1.777063294,1.779038557,1.781015703,
		1.782994735,1.784975653,1.786958459,1.788943154,1.790929739,1.792918216,1.794908586,1.796900851,1.798895012,1.800891070,1.802889026,1.804888882,1.806890640,1.808894300,1.810899864,1.812907334,1.814916710,1.816927994,1.818941188,1.820956293,
		1.822973309,1.824992240,1.827013085,1.829035846,1.831060525,1.833087123,1.835115641,1.837146080,1.839178443,1.841212730,1.843248943,1.845287083,1.847327152,1.849369150,1.851413080,1.853458942,1.855506738,1.857556469,1.859608137,1.861661744,
		1.863717289,1.865774775,1.867834204,1.869895575,1.871958892,1.874024155,1.876091366,1.878160525,1.880231635,1.882304697,1.884379711,1.886456680,1.888535605,1.890616488,1.892699328,1.894784129,1.896870891,1.898959615,1.901050304,1.903142958,
		1.905237579,1.907334168,1.909432726,1.911533256,1.913635757,1.915740233,1.917846683,1.919955110,1.922065515,1.924177898,1.926292263,1.928408609,1.930526939,1.932647253,1.934769553,1.936893841,1.939020117,1.941148384,1.943278642,1.945410893,
		1.947545139,1.949681380,1.951819618,1.953959855,1.956102092,1.958246329,1.960392570,1.962540814,1.964691064,1.966843321,1.968997585,1.971153859,1.973312145,1.975472442,1.977634753,1.979799079,1.981965422,1.984133782,1.986304162,1.988476562,
		1.990650984,1.992827430,1.995005900,1.997186397,1.999368921,2.001553474,2.003740057,2.005928673,2.008119321,2.010312003,2.012506722,2.014703478,2.016902272,2.019103107,2.021305982,2.023510901,2.025717864,2.027926873,2.030137928,2.032351033,
		2.034566186,2.036783391,2.039002649,2.041223961,2.043447328,2.045672751,2.047900233,2.050129775,2.052361377,2.054595042,2.056830770,2.059068564,2.061308424,2.063550352,2.065794350,2.068040418,2.070288559,2.072538773,2.074791061,2.077045427,
		2.079301870,2.081560392,2.083820994,2.086083679,2.088348447,2.090615300,2.092884238,2.095155265,2.097428380,2.099703586,2.101980883,2.104260274,2.106541759,2.108825340,2.111111019,2.113398796,2.115688674,2.117980653,2.120274735,2.122570922,
		2.124869214,2.127169614,2.129472122,2.131776740,2.134083470,2.136392313,2.138703269,2.141016342,2.143331532,2.145648840,2.147968268,2.150289817,2.152613489,2.154939285,2.157267207,2.159597256,2.161929433,2.164263740,2.166600178,2.168938749,
		2.171279454,2.173622294,2.175967271,2.178314386,2.180663641,2.183015037,2.185368576,2.187724259,2.190082087,2.192442061,2.194804184,2.197168457,2.199534880,2.201903456,2.204274186,2.206647071,2.209022113,2.211399313,2.213778673,2.216160193,
		2.218543876,2.220929723,2.223317735,2.225707913,2.228100260,2.230494776,2.232891463,2.235290322,2.237691355,2.240094563,2.242499948,2.244907511,2.247317253,2.249729177,2.252143282,2.254559571,2.256978046,2.259398707,2.261821556,2.264246594,
		2.266673824,2.269103245,2.271534860,2.273968671,2.276404678,2.278842883,2.281283287,2.283725893,2.286170700,2.288617712,2.291066928,2.293518351,2.295971982,2.298427823,2.300885874,2.303346138,2.305808615,2.308273308,2.310740217,2.313209344,
		2.315680691,2.318154259,2.320630049,2.323108063,2.325588302,2.328070767,2.330555461,2.333042384,2.335531539,2.338022925,2.340516546,2.343012401,2.345510494,2.348010825,2.350513395,2.353018206,2.355525260,2.358034557,2.360546100,2.363059890,
		2.365575928,2.368094215,2.370614754,2.373137545,2.375662591,2.378189891,2.380719449,2.383251265,2.385785340,2.388321677,2.390860277,2.393401140,2.395944269,2.398489665,2.401037330,2.403587264,2.406139470,2.408693949,2.411250701,2.413809730,
		2.416371035,2.418934619,2.421500483,2.424068628,2.426639057,2.429211770,2.431786768,2.434364054,2.436943628,2.439525493,2.442109649,2.444696098,2.447284842,2.449875882,2.452469219,2.455064855,2.457662791,2.460263029,2.462865570,2.465470416,
		2.468077568,2.470687027,2.473298796,2.475912875,2.478529266,2.481147970,2.483768989,2.486392325,2.489017978,2.491645951,2.494276244,2.496908859,2.499543798,2.502181062,2.504820652,2.507462571,2.510106818,2.512753397,2.515402308,2.518053553,
		2.520707133,2.523363050,2.526021305,2.528681899,2.531344835,2.534010113,2.536677736,2.539347704,2.542020018,2.544694681,2.547371695,2.550051059,2.552732776,2.555416848,2.558103275,2.560792059,2.563483202,2.566176706,2.568872570,2.571570798,
		2.574271391,2.576974349,2.579679675,2.582387370,2.585097435,2.587809872,2.590524683,2.593241868,2.595961429,2.598683369,2.601407687,2.604134386,2.606863468,2.609594933,2.612328783,2.615065020,2.617803644,2.620544658,2.623288064,2.626033861,
		2.628782053,2.631532640,2.634285624,2.637041006,2.639798788,2.642558972,2.645321558,2.648086548,2.650853944,2.653623748,2.656395960,2.659170582,2.661947616,2.664727063,2.667508925,2.670293202,2.673079897,2.675869012,2.678660546,2.681454503,
		2.684250883,2.687049689,2.689850920,2.692654580,2.695460669,2.698269188,2.701080141,2.703893527,2.706709348,2.709527606,2.712348302,2.715171438,2.717997016,2.720825036,2.723655500,2.726488410,2.729323768,2.732161573,2.735001829,2.737844537,
		2.740689698,2.743537314,2.746387385,2.749239914,2.752094902,2.754952351,2.757812262,2.760674637,2.763539476,2.766406782,2.769276556,2.772148799,2.775023514,2.777900701,2.780780362,2.783662498,2.786547111,2.789434203,2.792323775,2.795215828,
		2.798110364,2.801007384,2.803906891,2.806808885,2.809713367,2.812620340,2.815529805,2.818441764,2.821356217,2.824273166,2.827192614,2.830114561,2.833039008,2.835965958,2.838895412,2.841827371,2.844761837,2.847698812,2.850638296,2.853580292,
		2.856524800,2.859471823,2.862421361,2.865373417,2.868327992,2.871285087,2.874244704,2.877206844,2.880171510,2.883138701,2.886108420,2.889080669,2.892055449,2.895032760,2.898012606,2.900994987,2.903979905,2.906967362,2.909957358,2.912949895,
		2.915944976,2.918942601,2.921942771,2.924945490,2.927950757,2.930958574,2.933968944,2.936981866,2.939997344,2.943015378,2.946035971,2.949059122,2.952084835,2.955113110,2.958143949,2.961177354,2.964213325,2.967251865,2.970292976,2.973336657,
		2.976382912,2.979431741,2.982483147,2.985537130,2.988593692,2.991652835,2.994714560,2.997778868,3.000845762,3.003915242,3.006987311,3.010061969,3.013139219,3.016219061,3.019301497,3.022386529,3.025474159,3.028564387,3.031657216,3.034752647,
		3.037850680,3.040951319,3.044054564,3.047160417,3.050268880,3.053379953,3.056493639,3.059609939,3.062728854,3.065850387,3.068974538,3.072101309,3.075230701,3.078362717,3.081497357,3.084634624,3.087774518,3.090917042,3.094062196,3.097209982,
		3.100360403,3.103513458,3.106669150,3.109827481,3.112988452,3.116152064,3.119318319,3.122487218,3.125658764,3.128832956,3.132009798,3.135189291,3.138371435,3.141556234,3.144743687,3.147933797,3.151126565,3.154321993,3.157520083,3.160720835,
		3.163924251,3.167130333,3.170339083,3.173550502,3.176764591,3.179981352,3.183200786,3.186422896,3.189647682,3.192875146,3.196105290,3.199338115,3.202573623,3.205811815,3.209052693,3.212296259,3.215542513,3.218791457,3.222043094,3.225297423,
		3.228554448,3.231814170,3.235076589,3.238341708,3.241609529,3.244880052,3.248153279,3.251429212,3.254707852,3.257989201,3.261273261,3.264560032,3.267849517,3.271141717,3.274436634,3.277734268,3.281034622,3.284337698,3.287643496,3.290952018,
		3.294263267,3.297577242,3.300893946,3.304213381,3.307535548,3.310860449,3.314188084,3.317518456,3.320851566,3.324187416,3.327526007,3.330867341,3.334211420,3.337558244,3.340907815,3.344260136,3.347615207,3.350973030,3.354333607,3.357696939,
		3.361063028,3.364431875,3.367803482,3.371177850,3.374554981,3.377934877,3.381317539,3.384702968,3.388091166,3.391482135,3.394875877,3.398272392,3.401671682,3.405073749,3.408478595,3.411886221,3.415296628,3.418709819,3.422125794,3.425544555,
		3.428966104,3.432390443,3.435817572,3.439247494,3.442680209,3.446115721,3.449554029,3.452995136,3.456439043,3.459885753,3.463335265,3.466787582,3.470242706,3.473700638,3.477161379,3.480624932,3.484091297,3.487560477,3.491032472,3.494507285,
		3.497984916,3.501465368,3.504948643,3.508434740,3.511923663,3.515415413,3.518909991,3.522407399,3.525907639,3.529410711,3.532916618,3.536425361,3.539936942,3.543451362,3.546968623,3.550488727,3.554011674,3.557537467,3.561066107,3.564597595,
		3.568131934,3.571669125,3.575209169,3.578752068,3.582297823,3.585846437,3.589397910,3.592952244,3.596509442,3.600069503,3.603632431,3.607198226,3.610766890,3.614338424,3.617912831,3.621490112,3.625070268,3.628653301,3.632239212,3.635828004,
		3.639419677,3.643014234,3.646611675,3.650212002,3.653815218,3.657421323,3.661030319,3.664642208,3.668256991,3.671874670,3.675495246,3.679118722,3.682745097,3.686374375,3.690006557,3.693641644,3.697279638,3.700920540,3.704564352,3.708211076,
		3.711860713,3.715513265,3.719168733,3.722827119,3.726488425,3.730152652,3.733819801,3.737489875,3.741162874,3.744838801,3.748517657,3.752199443,3.755884162,3.759571814,3.763262402,3.766955927,3.770652390,3.774351793,3.778054138,3.781759426,
		3.785467659,3.789178838,3.792892966,3.796610043,3.800330071,3.804053052,3.807778987,3.811507878,3.815239727,3.818974535,3.822712304,3.826453035,3.830196729,3.833943390,3.837693017,3.841445613,3.845201180,3.848959718,3.852721229,3.856485716,
		3.860253179,3.864023620,3.867797042,3.871573444,3.875352830,3.879135200,3.882920556,3.886708901,3.890500234,3.894294559,3.898091876,3.901892187,3.905695494,3.909501798,3.913311101,3.917123405,3.920938710,3.924757020,3.928578335,3.932402657,
		3.936229987,3.940060327,3.943893679,3.947730045,3.951569425,3.955411822,3.959257237,3.963105671,3.966957127,3.970811606,3.974669110,3.978529639,3.982393196,3.986259783,3.990129400,3.994002050,3.997877734,4.001756454,4.005638211,4.009523007,
		4.013410844,4.017301722,4.021195645,4.025092612,4.028992627,4.032895690,4.036801804,4.040710969,4.044623188,4.048538461,4.052456791,4.056378180,4.060302628,4.064230137,4.068160710,4.072094347,4.076031050,4.079970822,4.083913662,4.087859574,
		4.091808558,4.095760617,4.099715752,4.103673964,4.107635255,4.111599627,4.115567081,4.119537619,4.123511243,4.127487954,4.131467754,4.135450644,4.139436626,4.143425701,4.147417872,4.151413140,4.155411506,4.159412973,4.163417541,4.167425212,
		4.171435988,4.175449871,4.179466862,4.183486963,4.187510175,4.191536500,4.195565940,4.199598496,4.203634169,4.207672963,4.211714877,4.215759914,4.219808076,4.223859363,4.227913778,4.231971322,4.236031997,4.240095805,4.244162746,4.248232823,
		4.252306038,4.256382391,4.260461885,4.264544521,4.268630301,4.272719226,4.276811298,4.280906519,4.285004890,4.289106413,4.293211090,4.297318922,4.301429910,4.305544057,4.309661364,4.313781833,4.317905465,4.322032262,4.326162225,4.330295357,
		4.334431658,4.338571131,4.342713777,4.346859597,4.351008594,4.355160768,4.359316123,4.363474658,4.367636376,4.371801279,4.375969368,4.380140644,4.384315110,4.388492767,4.392673616,4.396857660,4.401044899,4.405235336,4.409428972,4.413625809,
		4.417825849,4.422029092,4.426235541,4.430445197,4.434658062,4.438874138,4.443093426,4.447315928,4.451541645,4.455770580,4.460002733,4.464238106,4.468476702,4.472718521,4.476963566,4.481211837,4.485463337,4.489718067,4.493976029,4.498237225,
		4.502501655,4.506769323,4.511040229,4.515314374,4.519591762,4.523872393,4.528156269,4.532443391,4.536733762,4.541027382,4.545324255,4.549624380,4.553927760,4.558234396,4.562544291,4.566857445,4.571173861,4.575493540,4.579816484,4.584142693,
		4.588472171,4.592804919,4.597140937,4.601480229,4.605822795,4.610168637,4.614517757,4.618870157,4.623225838,4.627584801,4.631947049,4.636312583,4.640681405,4.645053516,4.649428919,4.653807613,4.658189603,4.662574888,4.666963471,4.671355353,
		4.675750536,4.680149021,4.684550811,4.688955907,4.693364310,4.697776022,4.702191046,4.706609381,4.711031031,4.715455997,4.719884280,4.724315882,4.728750805,4.733189051,4.737630620,4.742075515,4.746523738,4.750975289,4.755430172,4.759888386,
		4.764349935,4.768814819,4.773283041,4.777754601,4.782229503,4.786707746,4.791189333,4.795674267,4.800162547,4.804654176,4.809149156,4.813647488,4.818149174,4.822654215,4.827162614,4.831674372,4.836189490,4.840707970,4.845229815,4.849755024,
		4.854283601,4.858815547,4.863350864,4.867889552,4.872431614,4.876977052,4.881525867,4.886078061,4.890633636,4.895192592,4.899754933,4.904320659,4.908889772,4.913462274,4.918038166,4.922617450,4.927200129,4.931786203,4.936375674,4.940968544,
		4.945564814,4.950164486,4.954767563,4.959374045,4.963983934,4.968597232,4.973213940,4.977834061,4.982457595,4.987084545,4.991714912,4.996348698,5.000985905,5.005626533,5.010270586,5.014918064,5.019568970,5.024223304,5.028881069,5.033542266,
		5.038206897,5.042874964,5.047546468,5.052221411,5.056899794,5.061581620,5.066266890,5.070955605,5.075647768,5.080343380,5.085042442,5.089744957,5.094450925,5.099160350,5.103873232,5.108589573,5.113309374,5.118032638,5.122759367,5.127489561,
		5.132223222,5.136960352,5.141700954,5.146445028,5.151192576,5.155943600,5.160698101,5.165456082,5.170217543,5.174982487,5.179750916,5.184522830,5.189298232,5.194077123,5.198859506,5.203645381,5.208434750,5.213227615,5.218023978,5.222823841,
		5.227627205,5.232434071,5.237244442,5.242058319,5.246875704,5.251696598,5.256521004,5.261348923,5.266180356,5.271015305,5.275853773,5.280695760,5.285541268,5.290390300,5.295242856,5.300098939,5.304958549,5.309821690,5.314688362,5.319558567,
		5.324432307,5.329309584,5.334190399,5.339074754,5.343962650,5.348854090,5.353749075,5.358647606,5.363549686,5.368455316,5.373364498,5.378277233,5.383193524,5.388113371,5.393036777,5.397963743,5.402894271,5.407828363,5.412766020,5.417707244,
		5.422652037,5.427600401,5.432552336,5.437507845,5.442466930,5.447429592,5.452395833,5.457365655,5.462339059,5.467316047,5.472296621,5.477280782,5.482268532,5.487259873,5.492254807,5.497253335,5.502255459,5.507261180,5.512270501,5.517283423,
		5.522299947,5.527320076,5.532343811,5.537371154,5.542402107,5.547436670,5.552474847,5.557516638,5.562562046,5.567611072,5.572663717,5.577719984,5.582779875,5.587843390,5.592910531,5.597981301,5.603055701,5.608133733,5.613215398,5.618300698,
		5.623389635,5.628482211,5.633578427,5.638678285,5.643781786,5.648888933,5.653999727,5.659114170,5.664232263,5.669354008,5.674479408,5.679608463,5.684741175,5.689877547,5.695017579,5.700161274,5.705308633,5.710459657,5.715614350,5.720772712,
		5.725934744,5.731100450,5.736269830,5.741442886,5.746619620,5.751800034,5.756984129,5.762171907,5.767363370,5.772558520,5.777757357,5.782959885,5.788166104,5.793376017,5.798589624,5.803806928,5.809027931,5.814252634,5.819481039,5.824713148,
		5.829948962,5.835188483,5.840431713,5.845678654,5.850929306,5.856183673,5.861441755,5.866703555,5.871969074,5.877238314,5.882511276,5.887787963,5.893068376,5.898352516,5.903640386,5.908931987,5.914227321,5.919526390,5.924829195,5.930135738,
		5.935446021,5.940760045,5.946077813,5.951399326,5.956724585,5.962053593,5.967386351,5.972722861,5.978063124,5.983407143,5.988754919,5.994106454,5.999461749,6.004820807,6.010183628,6.015550215,6.020920570,6.026294694,6.031672589,6.037054256,
		6.042439698,6.047828915,6.053221911,6.058618686,6.064019243,6.069423582,6.074831706,6.080243617,6.085659316,6.091078805,6.096502085,6.101929159,6.107360028,6.112794694,6.118233159,6.123675424,6.129121490,6.134571361,6.140025038,6.145482521,
		6.150943814,6.156408917,6.161877833,6.167350563,6.172827109,6.178307473,6.183791656,6.189279660,6.194771488,6.200267140,6.205766618,6.211269924,6.216777061,6.222288029,6.227802830,6.233321466,6.238843939,6.244370251,6.249900403,6.255434397,
		6.260972235,6.266513919,6.272059449,6.277608829,6.283162059,6.288719142,6.294280079,6.299844872,6.305413523,6.310986034,6.316562405,6.322142640,6.327726739,6.333314705,6.338906538,6.344502242,6.350101818,6.355705267,6.361312591,6.366923792,
		6.372538871,6.378157831,6.383780674,6.389407400,6.395038011,6.400672511,6.406310899,6.411953178,6.417599350,6.423249416,6.428903379,6.434561239,6.440222999,6.445888660,6.451558225,6.457231694,6.462909070,6.468590355,6.474275549,6.479964656,
		6.485657676,6.491354612,6.497055464,6.502760236,6.508468928,6.514181543,6.519898082,6.525618547,6.531342939,6.537071261,6.542803514,6.548539700,6.554279821,6.560023878,6.565771874,6.571523809,6.577279686,6.583039507,6.588803273,6.594570986,
		6.600342647,6.606118259,6.611897824,6.617681342,6.623468816,6.629260248,6.635055639,6.640854991,6.646658306,6.652465586,6.658276831,6.664092045,6.669911229,6.675734385,6.681561513,6.687392617,6.693227698,6.699066758,6.704909798,6.710756820,
		6.716607826,6.722462818,6.728321797,6.734184766,6.740051725,6.745922678,6.751797625,6.757676568,6.763559509,6.769446450,6.775337393,6.781232339,6.787131291,6.793034249,6.798941216,6.804852193,6.810767182,6.816686186,6.822609205,6.828536241,
		6.834467297,6.840402374,6.846341474,6.852284599,6.858231749,6.864182928,6.870138137,6.876097377,6.882060651,6.888027960,6.893999306,6.899974691,6.905954116,6.911937584,6.917925095,6.923916653,6.929912258,6.935911912,6.941915618,6.947923376,
		6.953935190,6.959951059,6.965970987,6.971994975,6.978023025,6.984055138,6.990091317,6.996131563,7.002175877,7.008224263,7.014276721,7.020333253,7.026393861,7.032458547,7.038527312,7.044600159,7.050677089,7.056758103,7.062843205,7.068932395,
		7.075025675,7.081123047,7.087224513,7.093330074,7.099439733,7.105553491,7.111671350,7.117793311,7.123919377,7.130049549,7.136183830,7.142322220,7.148464722,7.154611337,7.160762067,7.166916914,7.173075880,7.179238967,7.185406175,7.191577508,
		7.197752967,7.203932553,7.210116269,7.216304116,7.222496096,7.228692211,7.234892463,7.241096852,7.247305382,7.253518054,7.259734870,7.265955831,7.272180939,7.278410197,7.284643605,7.290881167,7.297122882,7.303368754,7.309618784,7.315872973,
		7.322131325,7.328393839,7.334660519,7.340931366,7.347206382,7.353485568,7.359768927,7.366056459,7.372348168,7.378644055,7.384944120,7.391248368,7.397556798,7.403869413,7.410186215,7.416507206,7.422832387,7.429161760,7.435495326,7.441833089,
		7.448175049,7.454521208,7.460871568,7.467226131,7.473584899,7.479947873,7.486315056,7.492686448,7.499062052,7.505441870,7.511825904,7.518214154,7.524606624,7.531003315,7.537404228,7.543809365,7.550218729,7.556632321,7.563050143,7.569472196,
		7.575898483,7.582329005,7.588763763,7.595202761,7.601646000,7.608093481,7.614545206,7.621001177,7.627461396,7.633925865,7.640394585,7.646867558,7.653344787,7.659826272,7.666312016,7.672802020,7.679296287,7.685794818,7.692297614,7.698804679,
		7.705316013,7.711831618,7.718351496,7.724875649,7.731404079,7.737936788,7.744473776,7.751015047,7.757560602,7.764110443,7.770664571,7.777222989,7.783785697,7.790352699,7.796923996,7.803499589,7.810079480,7.816663672,7.823252166,7.829844964,
		7.836442067,7.843043478,7.849649199,7.856259230,7.862873574,7.869492233,7.876115209,7.882742503,7.889374117,7.896010053,7.902650312,7.909294898,7.915943811,7.922597053,7.929254626,7.935916532,7.942582773,7.949253350,7.955928265,7.962607521,
		7.969291119,7.975979060,7.982671347,7.989367981,7.996068965,8.002774299,8.009483987,8.016198029,8.022916428,8.029639185,8.036366302,8.043097781,8.049833623,8.056573832,8.063318407,8.070067352,8.076820668,8.083578357,8.090340421,8.097106861,
		8.103877679,8.110652878,8.117432459,8.124216423,8.131004773,8.137797510,8.144594637,8.151396155,8.158202066,8.165012371,8.171827073,8.178646173,8.185469674,8.192297577,8.199129883,8.205966595,8.212807715,8.219653244,8.226503184,8.233357538,
		8.240216306,8.247079490,8.253947094,8.260819117,8.267695563,8.274576432,8.281461728,8.288351451,8.295245603,8.302144187,8.309047204,8.315954655,8.322866544,8.329782871,8.336703639,8.343628848,8.350558502,8.357492602,8.364431150,8.371374147,
		8.378321595,8.385273497,8.392229854,8.399190668,8.406155940,8.413125673,8.420099869,8.427078529,8.434061655,8.441049248,8.448041312,8.455037847,8.462038856,8.469044339,8.476054300,8.483068740,8.490087661,8.497111064,8.504138952,8.511171325,
		8.518208187,8.525249539,8.532295383,8.539345721,8.546400553,8.553459884,8.560523713,8.567592043,8.574664876,8.581742214,8.588824058,8.595910411,8.603001274,8.610096649,8.617196537,8.624300942,8.631409864,8.638523305,8.645641268,8.652763753,
		8.659890764,8.667022301,8.674158367,8.681298964,8.688444092,8.695593755,8.702747954,8.709906690,8.717069967,8.724237785,8.731410146,8.738587052,8.745768505,8.752954508,8.760145061,8.767340166,8.774539826,8.781744043,8.788952817,8.796166152,
		8.803384048,8.810606508,8.817833533,8.825065126,8.832301288,8.839542021,8.846787327,8.854037208,8.861291665,8.868550701,8.875814318,8.883082516,8.890355298,8.897632666,8.904914622,8.912201168,8.919492305,8.926788035,8.934088360,8.941393283,
		8.948702804,8.956016926,8.963335650,8.970658979,8.977986914,8.985319457,8.992656610,8.999998375,9.007344754,9.014695748,9.022051359,9.029411590,9.036776442,9.044145917,9.051520016,9.058898742,9.066282097,9.073670082,9.081062699,9.088459951,
		9.095861838,9.103268363,9.110679528,9.118095335,9.125515784,9.132940879,9.140370621,9.147805012,9.155244054,9.162687749,9.170136098,9.177589103,9.185046767,9.192509091,9.199976077,9.207447727,9.214924042,9.222405025,9.229890678,9.237381001,
		9.244875998,9.252375670,9.259880019,9.267389047,9.274902755,9.282421146,9.289944221,9.297471982,9.305004431,9.312541571,9.320083402,9.327629926,9.335181147,9.342737064,9.350297681,9.357863000,9.365433021,9.373007747,9.380587180,9.388171321,
		9.395760173,9.403353737,9.410952016,9.418555010,9.426162723,9.433775155,9.441392309,9.449014187,9.456640790,9.464272120,9.471908179,9.479548970,9.487194494,9.494844752,9.502499747,9.510159480,9.517823954,9.525493171,9.533167131,9.540845837,
		9.548529292,9.556217496,9.563910451,9.571608161,9.579310625,9.587017847,9.594729828,9.602446571,9.610168076,9.617894345,9.625625382,9.633361187,9.641101762,9.648847110,9.656597232,9.664352130,9.672111805,9.679876261,9.687645498,9.695419519,
		9.703198325,9.710981919,9.718770302,9.726563476,9.734361442,9.742164204,9.749971762,9.757784119,9.765601276,9.773423236,9.781250000,9.789081570,9.796917948,9.804759136,9.812605136,9.820455949,9.828311578,9.836172024,9.844037289,9.851907376,
		9.859782286,9.867662021,9.875546582,9.883435972,9.891330193,9.899229246,9.907133134,9.915041858,9.922955420,9.930873822,9.938797066,9.946725154,9.954658087,9.962595868,9.970538499,9.978485980,9.986438315,9.994395506,10.002357553,10.010324459,
		10.018296226,10.026272855,10.034254349,10.042240710,10.050231938,10.058228038,10.066229009,10.074234854,10.082245575,10.090261174,10.098281653,10.106307013,10.114337256,10.122372385,10.130412402,10.138457307,10.146507103,10.154561793,10.162621377,10.170685858,
		10.178755237,10.186829517,10.194908700,10.202992787,10.211081780,10.219175681,10.227274492,10.235378214,10.243486851,10.251600403,10.259718873,10.267842262,10.275970573,10.284103807,10.292241966,10.300385052,10.308533067,10.316686013,10.324843892,10.333006705,
		10.341174455,10.349347144,10.357524772,10.365707343,10.373894858,10.382087319,10.390284728,10.398487087,10.406694398,10.414906662,10.423123882,10.431346059,10.439573195,10.447805293,10.456042354,10.464284380,10.472531373,10.480783335,10.489040268,10.497302173,
		10.505569053,10.513840909,10.522117744,10.530399559,10.538686356,10.546978137,10.555274905,10.563576660,10.571883405,10.580195142,10.588511872,10.596833598,10.605160321,10.613492044,10.621828768,10.630170495,10.638517227,10.646868966,10.655225714,10.663587473,
		10.671954245,10.680326031,10.688702834,10.697084655,10.705471496,10.713863360,10.722260248,10.730662162,10.739069104,10.747481076,10.755898079,10.764320117,10.772747190,10.781179300,10.789616450,10.798058641,10.806505876,10.814958156,10.823415483,10.831877859,
		10.840345286,10.848817765,10.857295300,10.865777891,10.874265541,10.882758251,10.891256024,10.899758861,10.908266764,10.916779736,10.925297778,10.933820891,10.942349079,10.950882342,10.959420683,10.967964104,10.976512606,10.985066192,10.993624864,11.002188622,
		11.010757470,11.019331409,11.027910442,11.036494569,11.045083793,11.053678116,11.062277540,11.070882067,11.079491698,11.088106436,11.096726282,11.105351238,11.113981307,11.122616491,11.131256790,11.139902207,11.148552745,11.157208404,11.165869187,11.174535096,
		11.183206133,11.191882299,11.200563597,11.209250029,11.217941595,11.226638299,11.235340143,11.244047127,11.252759255,11.261476527,11.270198947,11.278926515,11.287659235,11.296397107,11.305140133,11.313888316,11.322641658,11.331400160,11.340163825,11.348932654,
		11.357706649,11.366485812,11.375270146,11.384059651,11.392854330,11.401654185,11.410459218,11.419269431,11.428084826,11.436905404,11.445731167,11.454562118,11.463398259,11.472239590,11.481086115,11.489937836,11.498794753,11.507656869,11.516524187,11.525396707,
		11.534274432,11.543157364,11.552045505,11.560938856,11.569837420,11.578741199,11.587650194,11.596564407,11.605483841,11.614408497,11.623338377,11.632273483,11.641213818,11.650159382,11.659110179,11.668066209,11.677027475,11.685993979,11.694965723,11.703942708,
		11.712924937,11.721912412,11.730905133,11.739903105,11.748906327,11.757914803,11.766928535,11.775947523,11.784971770,11.794001279,11.803036050,11.812076087,11.821121390,11.830171962,11.839227805,11.848288921,11.857355311,11.866426978,11.875503923,11.884586149,
		11.893673657,11.902766450,11.911864529,11.920967896,11.930076553,11.939190503,11.948309747,11.957434286,11.966564124,11.975699261,11.984839701,11.993985444,12.003136493,12.012292850,12.021454516,12.030621494,12.039793785,12.048971392,12.058154317,12.067342561,
		12.076536126,12.085735014,12.094939228,12.104148769,12.113363639,12.122583840,12.131809375,12.141040244,12.150276450,12.159517995,12.168764881,12.178017110,12.187274683,12.196537603,12.205805872,12.215079491,12.224358463,12.233642790,12.242932473,12.252227514,
		12.261527916,12.270833680,12.280144808,12.289461303,12.298783165,12.308110398,12.317443003,12.326780982,12.336124337,12.345473070,12.354827183,12.364186678,12.373551557,12.382921821,12.392297473,12.401678515,12.411064949,12.420456776,12.429853998,12.439256619,
		12.448664638,12.458078060,12.467496884,12.476921114,12.486350752,12.495785798,12.505226256,12.514672127,12.524123413,12.533580117,12.543042239,12.552509783,12.561982749,12.571461141,12.580944959,12.590434206,12.599928885,12.609428996,12.618934541,12.628445524,
		12.637961945,12.647483807,12.657011112,12.666543861,12.676082057,12.685625701,12.695174796,12.704729343,12.714289345,12.723854803,12.733425719,12.743002096,12.752583935,12.762171239,12.771764008,12.781362246,12.790965954,12.800575134,12.810189788,12.819809918,
		12.829435526,12.839066615,12.848703185,12.858345239,12.867992779,12.877645807,12.887304325,12.896968334,12.906637838,12.916312837,12.925993334,12.935679330,12.945370828,12.955067830,12.964770338,12.974478353,12.984191877,12.993910913,13.003635463,13.013365528,
		13.023101111,13.032842213,13.042588836,13.052340983,13.062098655,13.071861854,13.081630583,13.091404843,13.101184636,13.110969965,13.120760830,13.130557235,13.140359181,13.150166670,13.159979704,13.169798285,13.179622416,13.189452097,13.199287331,13.209128121,
		13.218974467,13.228826372,13.238683838,13.248546867,13.258415461,13.268289622,13.278169351,13.288054652,13.297945525,13.307841973,13.317743997,13.327651601,13.337564785,13.347483552,13.357407903,13.367337841,13.377273368,13.387214485,13.397161195,13.407113500,
		13.417071401,13.427034901,13.437004001,13.446978704,13.456959011,13.466944925,13.476936447,13.486933580,13.496936325,13.506944685,13.516958660,13.526978255,13.537003470,13.547034307,13.557070768,13.567112856,13.577160572,13.587213918,13.597272897,13.607337510,
		13.617407759,13.627483647,13.637565175,13.647652345,13.657745159,13.667843620,13.677947728,13.688057487,13.698172898,13.708293963,13.718420685,13.728553064,13.738691104,13.748834806,13.758984172,13.769139204,13.779299904,13.789466275,13.799638317,13.809816033,
		13.819999426,13.830188497,13.840383247,13.850583680,13.860789797,13.871001600,13.881219090,13.891442271,13.901671144,13.911905711,13.922145974,13.932391935,13.942643596,13.952900959,13.963164026,13.973432799,13.983707280,13.993987471,14.004273373,14.014564990,
		14.024862323,14.035165374,14.045474145,14.055788638,14.066108854,14.076434797,14.086766468,14.097103868,14.107447001,14.117795867,14.128150470,14.138510810,14.148876891,14.159248713,14.169626279,14.180009592,14.190398652,14.200793462,14.211194024,14.221600340,
		14.232012412,14.242430242,14.252853833,14.263283185,14.273718301,14.284159183,14.294605833,14.305058252,14.315516444,14.325980410,14.336450152,14.346925672,14.357406972,14.367894054,14.378386920,14.388885572,14.399390012,14.409900242,14.420416265,14.430938081,
		14.441465693,14.451999103,14.462538314,14.473083326,14.483634143,14.494190765,14.504753196,14.515321437,14.525895490,14.536475357,14.547061040,14.557652542,14.568249863,14.578853007,14.589461975,14.600076769,14.610697391,14.621323844,14.631956129,14.642594248,
		14.653238203,14.663887996,14.674543630,14.685205106,14.695872426,14.706545593,14.717224608,14.727909473,14.738600191,14.749296763,14.759999191,14.770707478,14.781421625,14.792141634,14.802867508,14.813599248,14.824336857,14.835080337,14.845829688,14.856584915,
		14.867346018,14.878112999,14.888885861,14.899664605,14.910449234,14.921239750,14.932036154,14.942838449,14.953646637,14.964460719,14.975280698,14.986106576,14.996938355,15.007776036,15.018619622,15.029469115,15.040324516,15.051185829,15.062053054,15.072926194,
		15.083805251,15.094690227,15.105581124,15.116477944,15.127380689,15.138289361,15.149203962,15.160124493,15.171050959,15.181983359,15.192921696,15.203865972,15.214816190,15.225772351,15.236734457,15.247702510,15.258676512,15.269656466,15.280642373,15.291634236,
		15.302632055,15.313635835,15.324645575,15.335661279,15.346682949,15.357710586,15.368744192,15.379783770,15.390829322,15.401880849,15.412938354,15.424001839,15.435071305,15.446146755,15.457228191,15.468315614,15.479409028,15.490508433,15.501613832,15.512725227,
		15.523842620,15.534966013,15.546095408,15.557230807,15.568372212,15.579519625,15.590673048,15.601832483,15.612997932,15.624169398,15.635346882,15.646530386,15.657719913,15.668915464,15.680117041,15.691324647,15.702538283,15.713757951,15.724983655,15.736215394,
		15.747453173,15.758696992,15.769946853,15.781202759,15.792464712,15.803732714,15.815006767,15.826286872,15.837573032,15.848865250,15.860163526,15.871467863,15.882778263,15.894094728,15.905417261,15.916745862,15.928080535,15.939421281,15.950768102,15.962121001,
		15.973479978,15.984845038,15.996216180,16.007593408,16.018976724,16.030366129,16.041761626,16.053163216,16.064570902,16.075984686,16.087404569,16.098830555,16.110262644,16.121700839,16.133145142,16.144595555,16.156052079,16.167514718,16.178983474,16.190458347,
		16.201939340,16.213426456,16.224919696,16.236419062,16.247924556,16.259436181,16.270953939,16.282477831,16.294007859,16.305544027,16.317086334,16.328634785,16.340189380,16.351750122,16.363317013,16.374890055,16.386469249,16.398054599,16.409646106,16.421243771,
		16.432847598,16.444457588,16.456073743,16.467696066,16.479324558,16.490959221,16.502600058,16.514247070,16.525900260,16.537559629,16.549225180,16.560896914,16.572574835,16.584258943,16.595949241,16.607645731,16.619348415,16.631057294,16.642772372,16.654493650,
		16.666221130,16.677954815,16.689694705,16.701440804,16.713193113,16.724951635,16.736716371,16.748487324,16.760264496,16.772047888,16.783837502,16.795633342,16.807435408,16.819243703,16.831058230,16.842878989,16.854705983,16.866539214,16.878378685,16.890224397,
		16.902076352,16.913934552,16.925799000,16.937669697,16.949546646,16.961429848,16.973319306,16.985215022,16.997116997,17.009025235,17.020939736,17.032860503,17.044787538,17.056720843,17.068660420,17.080606272,17.092558400,17.104516806,17.116481492,17.128452461,
		17.140429715,17.152413255,17.164403083,17.176399202,17.188401615,17.200410321,17.212425325,17.224446628,17.236474232,17.248508139,17.260548351,17.272594870,17.284647698,17.296706838,17.308772292,17.320844060,17.332922147,17.345006553,17.357097280,17.369194331,
		17.381297709,17.393407414,17.405523449,17.417645816,17.429774517,17.441909554,17.454050930,17.466198646,17.478352704,17.490513106,17.502679856,17.514852953,17.527032402,17.539218203,17.551410359,17.563608871,17.575813743,17.588024976,17.600242572,17.612466532,
		17.624696861,17.636933558,17.649176627,17.661426069,17.673681887,17.685944082,17.698212657,17.710487614,17.722768955,17.735056681,17.747350795,17.759651300,17.771958196,17.784271487,17.796591174,17.808917260,17.821249745,17.833588634,17.845933926,17.858285626,
		17.870643734,17.883008253,17.895379185,17.907756532,17.920140295,17.932530478,17.944927083,17.957330110,17.969739563,17.982155443,17.994577753,18.007006494,18.019441669,18.031883279,18.044331328,18.056785816,18.069246747,18.081714121,18.094187942,18.106668210,
		18.119154930,18.131648101,18.144147727,18.156653810,18.169166351,18.181685353,18.194210817,18.206742747,18.219281143,18.231826009,18.244377346,18.256935156,18.269499441,18.282070204,18.294647446,18.307231170,18.319821377,18.332418070,18.345021251,18.357630922,
		18.370247085,18.382869741,18.395498894,18.408134546,18.420776697,18.433425351,18.446080510,18.458742175,18.471410349,18.484085034,18.496766231,18.509453944,18.522148173,18.534848922,18.547556192,18.560269985,18.572990303,18.585717149,18.598450525,18.611190432,
		18.623936873,18.636689850,18.649449365,18.662215420,18.674988017,18.687767158,18.700552846,18.713345082,18.726143868,18.738949207,18.751761101,18.764579552,18.777404561,18.790236131,18.803074265,18.815918963,18.828770229,18.841628064,18.854492471,18.867363451,
		18.880241007,18.893125140,18.906015853,18.918913149,18.931817028,18.944727493,18.957644547,18.970568190,18.983498427,18.996435257,19.009378685,19.022328711,19.035285337,19.048248567,19.061218402,19.074194844,19.087177894,19.100167557,19.113163832,19.126166723,
		19.139176232,19.152192360,19.165215111,19.178244485,19.191280485,19.204323113,19.217372371,19.230428261,19.243490786,19.256559948,19.269635748,19.282718189,19.295807272,19.308903001,19.322005376,19.335114401,19.348230076,19.361352405,19.374481390,19.387617032,
		19.400759334,19.413908297,19.427063924,19.440226217,19.453395179,19.466570810,19.479753114,19.492942092,19.506137746,19.519340079,19.532549093,19.545764789,19.558987171,19.572216239,19.585451997,19.598694446,19.611943588,19.625199425,19.638461960,19.651731195,
		19.665007132,19.678289772,19.691579118,19.704875173,19.718177937,19.731487414,19.744803606,19.758126514,19.771456140,19.784792488,19.798135558,19.811485353,19.824841876,19.838205127,19.851575110,19.864951826,19.878335278,19.891725467,19.905122397,19.918526068,
		19.931936483,19.945353644,19.958777554,19.972208213,19.985645625,19.999089792,20.012540716,20.025998398,20.039462841,20.052934047,20.066412019,20.079896757,20.093388265,20.106886545,20.120391598,20.133903427,20.147422033,20.160947420,20.174479589,20.188018541,
		20.201564281,20.215116808,20.228676126,20.242242237,20.255815143,20.269394845,20.282981347,20.296574650,20.310174756,20.323781667,20.337395386,20.351015914,20.364643255,20.378277409,20.391918379,20.405566167,20.419220775,20.432882206,20.446550461,20.460225543,
		20.473907454,20.487596195,20.501291770,20.514994179,20.528703426,20.542419512,20.556142440,20.569872211,20.583608828,20.597352292,20.611102607,20.624859773,20.638623794,20.652394672,20.666172407,20.679957004,20.693748463,20.707546787,20.721351977,20.735164037,
		20.748982968,20.762808773,20.776641453,20.790481010,20.804327448,20.818180767,20.832040970,20.845908059,20.859782037,20.873662905,20.887550665,20.901445320,20.915346872,20.929255323,20.943170674,20.957092929,20.971022089,20.984958157,20.998901134,21.012851023,
		21.026807825,21.040771544,21.054742180,21.068719737,21.082704216,21.096695620,21.110693950,21.124699209,21.138711399,21.152730522,21.166756580,21.180789575,21.194829510,21.208876386,21.222930206,21.236990972,21.251058685,21.265133349,21.279214965,21.293303535,
		21.307399062,21.321501548,21.335610994,21.349727403,21.363850777,21.377981118,21.392118429,21.406262711,21.420413967,21.434572198,21.448737407,21.462909596,21.477088768,21.491274923,21.505468065,21.519668196,21.533875317,21.548089431,21.562310541,21.576538647,
		21.590773752,21.605015859,21.619264970,21.633521086,21.647784210,21.662054344,21.676331490,21.690615650,21.704906826,21.719205022,21.733510238,21.747822476,21.762141740,21.776468031,21.790801351,21.805141702,21.819489087,21.833843507,21.848204966,21.862573464,
		21.876949005,21.891331590,21.905721221,21.920117900,21.934521631,21.948932414,21.963350252,21.977775148,21.992207102,22.006646119,22.021092198,22.035545344,22.050005557,22.064472840,22.078947196,22.093428626,22.107917132,22.122412717,22.136915382,22.151425131,
		22.165941964,22.180465885,22.194996895,22.209534997,22.224080193,22.238632484,22.253191873,22.267758362,22.282331954,22.296912650,22.311500452,22.326095363,22.340697385,22.355306520,22.369922771,22.384546138,22.399176626,22.413814234,22.428458967,22.443110825,
		22.457769812,22.472435929,22.487109178,22.501789562,22.516477082,22.531171742,22.545873542,22.560582486,22.575298575,22.590021811,22.604752197,22.619489735,22.634234427,22.648986275,22.663745281,22.678511448,22.693284777,22.708065271,22.722852932,22.737647762,
		22.752449763,22.767258938,22.782075288,22.796898816,22.811729523,22.826567413,22.841412487,22.856264747,22.871124196,22.885990835,22.900864668,22.915745695,22.930633919,22.945529343,22.960431968,22.975341796,22.990258831,23.005183073,23.020114526,23.035053191,
		23.049999070,23.064952166,23.079912481,23.094880016,23.109854775,23.124836759,23.139825970,23.154822411,23.169826083,23.184836990,23.199855133,23.214880513,23.229913135,23.244952999,23.260000107,23.275054463,23.290116067,23.305184923,23.320261032,23.335344397,
		23.350435020,23.365532902,23.380638046,23.395750455,23.410870130,23.425997074,23.441131288,23.456272776,23.471421538,23.486577578,23.501740897,23.516911498,23.532089382,23.547274553,23.562467011,23.577666760,23.592873801,23.608088137,23.623309770,23.638538701,
		23.653774934,23.669018470,23.684269312,23.699527461,23.714792920,23.730065691,23.745345776,23.760633177,23.775927897,23.791229938,23.806539301,23.821855990,23.837180005,23.852511350,23.867850027,23.883196037,23.898549384,23.913910068,23.929278093,23.944653460,
		23.960036171,23.975426230,23.990823637,24.006228396,24.021640508,24.037059975,24.052486800,24.067920985,24.083362532,24.098811443,24.114267720,24.129731366,24.145202383,24.160680772,24.176166537,24.191659679,24.207160201,24.222668104,24.238183391,24.253706063,
		24.269236125,24.284773576,24.300318420,24.315870659,24.331430294,24.346997329,24.362571765,24.378153604,24.393742849,24.409339502,24.424943564,24.440555039,24.456173929,24.471800234,24.487433959,24.503075104,24.518723673,24.534379667,24.550043088,24.565713939,
		24.581392222,24.597077938,24.612771091,24.628471683,24.644179714,24.659895189,24.675618109,24.691348475,24.707086291,24.722831559,24.738584280,24.754344457,24.770112092,24.785887188,24.801669745,24.817459768,24.833257257,24.849062215,24.864874645,24.880694547,
		24.896521926,24.912356782,24.928199118,24.944048936,24.959906239,24.975771028,24.991643306,25.007523074,25.023410336,25.039305093,25.055207347,25.071117102,25.087034358,25.102959118,25.118891384,25.134831159,25.150778444,25.166733242,25.182695555,25.198665386,
		25.214642735,25.230627607,25.246620002,25.262619923,25.278627372,25.294642352,25.310664864,25.326694911,25.342732495,25.358777618,25.374830282,25.390890490,25.406958244,25.423033546,25.439116397,25.455206801,25.471304760,25.487410275,25.503523350,25.519643985,
		25.535772184,25.551907948,25.568051280,25.584202181,25.600360655,25.616526703,25.632700328,25.648881531,25.665070316,25.681266683,25.697470636,25.713682176,25.729901306,25.746128028,25.762362343,25.778604256,25.794853766,25.811110878,25.827375592,25.843647912,
		25.859927838,25.876215375,25.892510523,25.908813284,25.925123662,25.941441659,25.957767276,25.974100515,25.990441380,26.006789872,26.023145993,26.039509745,26.055881132,26.072260154,26.088646814,26.105041115,26.121443059,26.137852647,26.154269882,26.170694767,
		26.187127302,26.203567492,26.220015337,26.236470840,26.252934004,26.269404830,26.285883320,26.302369478,26.318863304,26.335364802,26.351873973,26.368390820,26.384915345,26.401447550,26.417987437,26.434535009,26.451090268,26.467653215,26.484223854,26.500802186,
		26.517388214,26.533981939,26.550583364,26.567192492,26.583809324,26.600433863,26.617066110,26.633706069,26.650353741,26.667009128,26.683672233,26.700343058,26.717021605,26.733707876,26.750401874,26.767103601,26.783813059,26.800530250,26.817255176,26.833987840,
		26.850728244,26.867476390,26.884232280,26.900995917,26.917767302,26.934546438,26.951333328,26.968127972,26.984930375,27.001740537,27.018558461,27.035384149,27.052217604,27.069058827,27.085907822,27.102764589,27.119629132,27.136501452,27.153381552,27.170269434,
		27.187165100,27.204068553,27.220979794,27.237898826,27.254825651,27.271760272,27.288702690,27.305652908,27.322610927,27.339576751,27.356550381,27.373531820,27.390521070,27.407518132,27.424523011,27.441535706,27.458556221,27.475584559,27.492620720,27.509664708,
		27.526716524,27.543776172,27.560843652,27.577918968,27.595002121,27.612093114,27.629191949,27.646298628,27.663413153,27.680535527,27.697665753,27.714803831,27.731949764,27.749103555,27.766265206,27.783434719,27.800612096,27.817797340,27.834990452,27.852191435,
		27.869400291,27.886617023,27.903841632,27.921074121,27.938314493,27.955562748,27.972818890,27.990082921,28.007354843,28.024634658,28.041922368,28.059217976,28.076521484,28.093832894,28.111152208,28.128479430,28.145814559,28.163157600,28.180508555,28.197867424,
		28.215234212,28.232608919,28.249991549,28.267382103,28.284780584,28.302186994,28.319601335,28.337023609,28.354453819,28.371891967,28.389338055,28.406792085,28.424254060,28.441723981,28.459201852,28.476687674,28.494181449,28.511683180,28.529192869,28.546710519,
		28.564236130,28.581769707,28.599311250,28.616860763,28.634418246,28.651983704,28.669557137,28.687138549,28.704727941,28.722325315,28.739930675,28.757544021,28.775165357,28.792794684,28.810432005,28.828077322,28.845730637,28.863391953,28.881061271,28.898738595,
		28.916423925,28.934117266,28.951818618,28.969527983,28.987245365,29.004970766,29.022704187,29.040445631,29.058195100,29.075952597,29.093718123,29.111491682,29.129273274,29.147062903,29.164860570,29.182666278,29.200480029,29.218301826,29.236131670,29.253969564,
		29.271815510,29.289669511,29.307531568,29.325401684,29.343279861,29.361166101,29.379060407,29.396962781,29.414873225,29.432791741,29.450718331,29.468652999,29.486595745,29.504546573,29.522505484,29.540472481,29.558447566,29.576430742,29.594422010,29.612421372,
		29.630428832,29.648444391,29.666468052,29.684499816,29.702539687,29.720587665,29.738643755,29.756707957,29.774780274,29.792860708,29.810949262,29.829045938,29.847150738,29.865263664,29.883384718,29.901513904,29.919651222,29.937796676,29.955950267,29.974111999,
		29.992281872,30.010459890,30.028646054,30.046840367,30.065042831,30.083253448,30.101472221,30.119699152,30.137934243,30.156177496,30.174428914,30.192688499,30.210956253,30.229232179,30.247516278,30.265808553,30.284109006,30.302417639,30.320734455,30.339059457,
		30.357392645,30.375734023,30.394083592,30.412441355,30.430807315,30.449181473,30.467563832,30.485954394,30.504353161,30.522760136,30.541175320,30.559598717,30.578030328,30.596470155,30.614918201,30.633374468,30.651838959,30.670311675,30.688792619,30.707281793,
		30.725779200,30.744284841,30.762798719,30.781320837,30.799851196,30.818389798,30.836936647,30.855491743,30.874055091,30.892626691,30.911206546,30.929794658,30.948391030,30.966995663,30.985608561,31.004229725,31.022859158,31.041496862,31.060142838,31.078797091,
		31.097459620,31.116130430,31.134809522,31.153496899,31.172192562,31.190896514,31.209608758,31.228329295,31.247058128,31.265795259,31.284540690,31.303294424,31.322056463,31.340826809,31.359605465,31.378392432,31.397187713,31.415991310,31.434803226,31.453623463,
		31.472452023,31.491288908,31.510134120,31.528987663,31.547849538,31.566719747,31.585598292,31.604485177,31.623380403,31.642283973,31.661195888,31.680116151,31.699044765,31.717981731,31.736927052,31.755880730,31.774842768,31.793813167,31.812791930,31.831779060,
		31.850774557,31.869778426,31.888790668,31.907811285,31.926840280,31.945877655,31.964923411,31.983977553,32.003040081,32.022110998,32.041190306,32.060278008,32.079374105,32.098478601,32.117591497,32.136712796,32.155842499,32.174980610,32.194127131,32.213282063,
		32.232445409,32.251617171,32.270797353,32.289985955,32.309182980,32.328388430,32.347602309,32.366824617,32.386055358,32.405294533,32.424542145,32.443798196,32.463062689,32.482335625,32.501617008,32.520906838,32.540205119,32.559511853,32.578827042,32.598150689,
		32.617482795,32.636823363,32.656172395,32.675529894,32.694895862,32.714270300,32.733653212,32.753044600,32.772444466,32.791852812,32.811269640,32.830694954,32.850128754,32.869571044,32.889021825,32.908481100,32.927948872,32.947425142,32.966909913,32.986403187,
		33.005904966,33.025415253,33.044934050,33.064461360,33.083997183,33.103541524,33.123094384,33.142655765,33.162225670,33.181804101,33.201391060,33.220986549,33.240590572,33.260203129,33.279824224,33.299453859,33.319092036,33.338738757,33.358394024,33.378057841,
		33.397730209,33.417411130,33.437100607,33.456798642,33.476505237,33.496220395,33.515944118,33.535676408,33.555417267,33.575166699,33.594924704,33.614691286,33.634466447,33.654250188,33.674042513,33.693843424,33.713652922,33.733471010,33.753297691,33.773132967,
		33.792976840,33.812829312,33.832690385,33.852560063,33.872438347,33.892325239,33.912220743,33.932124859,33.952037591,33.971958940,33.991888910,34.011827501,34.031774718,34.051730561,34.071695034,34.091668138,34.111649876,34.131640250,34.151639262,34.171646915,
		34.191663211,34.211688153,34.231721742,34.251763981,34.271814872,34.291874417,34.311942620,34.332019481,34.352105004,34.372199191,34.392302044,34.412413565,34.432533756,34.452662621,34.472800161,34.492946378,34.513101275,34.533264855,34.553437118,34.573618069,
		34.593807708,34.614006039,34.634213064,34.654428784,34.674653203,34.694886322,34.715128144,34.735378672,34.755637907,34.775905851,34.796182508,34.816467879,34.836761967,34.857064774,34.877376302,34.897696554,34.918025532,34.938363238,34.958709675,34.979064844,
		34.999428749,35.019801391,35.040182773,35.060572897,35.080971765,35.101379380,35.121795745,35.142220860,35.162654729,35.183097354,35.203548738,35.224008882,35.244477788,35.264955461,35.285441900,35.305937109,35.326441091,35.346953847,35.367475379,35.388005691,
		35.408544784,35.429092661,35.449649324,35.470214775,35.490789017,35.511372052,35.531963882,35.552564509,35.573173937,35.593792166,35.614419200,35.635055041,35.655699691,35.676353153,35.697015428,35.717686520,35.738366430,35.759055160,35.779752714,35.800459093,
		35.821174299,35.841898336,35.862631205,35.883372908,35.904123449,35.924882828,35.945651050,35.966428115,35.987214026,36.008008786,36.028812397,36.049624861,36.070446180,36.091276357,36.112115395,36.132963294,36.153820059,36.174685691,36.195560192,36.216443564,
		36.237335811,36.258236934,36.279146936,36.300065819,36.320993585,36.341930237,36.362875776,36.383830207,36.404793529,36.425765747,36.446746862,36.467736876,36.488735792,36.509743613,36.530760340,36.551785976,36.572820523,36.593863984,36.614916360,36.635977655,
		36.657047870,36.678127008,36.699215072,36.720312062,36.741417983,36.762532835,36.783656622,36.804789346,36.825931009,36.847081613,36.868241161,36.889409656,36.910587098,36.931773491,36.952968838,36.974173140,36.995386399,37.016608619,37.037839801,37.059079947,
		37.080329061,37.101587144,37.122854198,37.144130227,37.165415232,37.186709216,37.208012181,37.229324129,37.250645063,37.271974984,37.293313896,37.314661801,37.336018701,37.357384598,37.378759495,37.400143393,37.421536296,37.442938206,37.464349125,37.485769055,
		37.507197998,37.528635958,37.550082936,37.571538935,37.593003956,37.614478003,37.635961078,37.657453182,37.678954319,37.700464491,37.721983699,37.743511947,37.765049236,37.786595570,37.808150950,37.829715378,37.851288858,37.872871391,37.894462979,37.916063626,
		37.937673333,37.959292102,37.980919937,38.002556839,38.024202811,38.045857854,38.067521972,38.089195167,38.110877441,38.132568796,38.154269235,38.175978760,38.197697374,38.219425078,38.241161875,38.262907768,38.284662759,38.306426849,38.328200042,38.349982340,
		38.371773745,38.393574260,38.415383886,38.437202627,38.459030484,38.480867460,38.502713557,38.524568777,38.546433123,38.568306598,38.590189203,38.612080941,38.633981815,38.655891826,38.677810976,38.699739270,38.721676707,38.743623292,38.765579026,38.787543912,
		38.809517951,38.831501147,38.853493502,38.875495018,38.897505697,38.919525542,38.941554555,38.963592738,38.985640094,39.007696625,39.029762334,39.051837223,39.073921293,39.096014548,39.118116990,39.140228622,39.162349444,39.184479461,39.206618674,39.228767085,
		39.250924698,39.273091514,39.295267535,39.317452764,39.339647204,39.361850857,39.384063724,39.406285809,39.428517113,39.450757640,39.473007391,39.495266369,39.517534576,39.539812015,39.562098687,39.584394596,39.606699743,39.629014131,39.651337763,39.673670640,
		39.696012765,39.718364140,39.740724768,39.763094651,39.785473792,39.807862192,39.830259854,39.852666781,39.875082974,39.897508437,39.919943171,39.942387179,39.964840464,39.987303026,40.009774870,40.032255997,40.054746410,40.077246111,40.099755102,40.122273386,
		40.144800965,40.167337841,40.189884017,40.212439496,40.235004279,40.257578368,40.280161767,40.302754478,40.325356502,40.347967843,40.370588502,40.393218483,40.415857787,40.438506416,40.461164374,40.483831662,40.506508283,40.529194239,40.551889533,40.574594167,
		40.597308143,40.620031463,40.642764131,40.665506148,40.688257516,40.711018239,40.733788318,40.756567756,40.779356555,40.802154718,40.824962246,40.847779143,40.870605410,40.893441051,40.916286066,40.939140460,40.962004233,40.984877389,41.007759930,41.030651858,
		41.053553176,41.076463885,41.099383989,41.122313489,41.145252388,41.168200689,41.191158394,41.214125504,41.237102023,41.260087953,41.283083296,41.306088055,41.329102231,41.352125828,41.375158848,41.398201292,41.421253164,41.444314466,41.467385200,41.490465369,
		41.513554974,41.536654019,41.559762505,41.582880435,41.606007812,41.629144637,41.652290914,41.675446643,41.698611829,41.721786473,41.744970578,41.768164146,41.791367179,41.814579679,41.837801650,41.861033093,41.884274011,41.907524406,41.930784281,41.954053638,
		41.977332479,42.000620807,42.023918624,42.047225932,42.070542734,42.093869033,42.117204829,42.140550127,42.163904928,42.187269235,42.210643050,42.234026376,42.257419214,42.280821567,42.304233438,42.327654829,42.351085742,42.374526180,42.397976145,42.421435640,
		42.444904666,42.468383226,42.491871323,42.515368960,42.538876137,42.562392858,42.585919125,42.609454941,42.633000308,42.656555228,42.680119703,42.703693736,42.727277330,42.750870487,42.774473209,42.798085498,42.821707357,42.845338789,42.868979795,42.892630378,
		42.916290540,42.939960285,42.963639614,42.987328529,43.011027033,43.034735128,43.058452817,43.082180103,43.105916987,43.129663471,43.153419559,43.177185253,43.200960554,43.224745466,43.248539991,43.272344131,43.296157888,43.319981266,43.343814266,43.367656890,
		43.391509142,43.415371023,43.439242536,43.463123683,43.487014467,43.510914890,43.534824954,43.558744662,43.582674017,43.606613019,43.630561673,43.654519980,43.678487943,43.702465565,43.726452846,43.750449791,43.774456400,43.798472678,43.822498626,43.846534246,
		43.870579540,43.894634513,43.918699164,43.942773498,43.966857516,43.990951221,44.015054616,44.039167701,44.063290481,44.087422957,44.111565132,44.135717008,44.159878588,44.184049873,44.208230867,44.232421572,44.256621990,44.280832123,44.305051974,44.329281545,
		44.353520839,44.377769859,44.402028605,44.426297082,44.450575290,44.474863234,44.499160914,44.523468334,44.547785496,44.572112402,44.596449055,44.620795456,44.645151610,44.669517516,44.693893180,44.718278601,44.742673784,44.767078730,44.791493442,44.815917922,
		44.840352173,44.864796197,44.889249996,44.913713573,44.938186930,44.962670070,44.987162995,45.011665707,45.036178209,45.060700503,45.085232592,45.109774477,45.134326162,45.158887649,45.183458940,45.208040038,45.232630945,45.257231663,45.281842194,45.306462542,
		45.331092709,45.355732696,45.380382507,45.405042144,45.429711609,45.454390905,45.479080033,45.503778997,45.528487799,45.553206441,45.577934926,45.602673255,45.627421432,45.652179459,45.676947338,45.701725072,45.726512663,45.751310113,45.776117426,45.800934602,
		45.825761645,45.850598557,45.875445341,45.900301998,45.925168532,45.950044944,45.974931238,45.999827415,46.024733478,46.049649429,46.074575271,46.099511007,46.124456637,46.149412166,46.174377595,46.199352927,46.224338164,46.249333308,46.274338363,46.299353330,
		46.324378212,46.349413011,46.374457729,46.399512370,46.424576935,46.449651427,46.474735849,46.499830202,46.524934489,46.550048712,46.575172875,46.600306979,46.625451026,46.650605020,46.675768962,46.700942856,46.726126702,46.751320505,46.776524266,46.801737987,
		46.826961672,46.852195321,46.877438939,46.902692527,46.927956088,46.953229624,46.978513137,47.003806631,47.029110106,47.054423567,47.079747014,47.105080452,47.130423881,47.155777305,47.181140725,47.206514145,47.231897567,47.257290992,47.282694425,47.308107866,
		47.333531318,47.358964784,47.384408267,47.409861768,47.435325290,47.460798836,47.486282407,47.511776007,47.537279637,47.562793301,47.588317000,47.613850737,47.639394514,47.664948334,47.690512200,47.716086113,47.741670076,47.767264091,47.792868162,47.818482290,
		47.844106477,47.869740727,47.895385041,47.921039423,47.946703874,47.972378396,47.998062993,48.023757667,48.049462420,48.075177254,48.100902172,48.126637177,48.152382270,48.178137455,48.203902733,48.229678108,48.255463581,48.281259155,48.307064832,48.332880615,
		48.358706506,48.384542508,48.410388623,48.436244854,48.462111202,48.487987671,48.513874262,48.539770979,48.565677823,48.591594798,48.617521905,48.643459146,48.669406526,48.695364044,48.721331705,48.747309511,48.773297464,48.799295566,48.825303820,48.851322228,
		48.877350794,48.903389518,48.929438404,48.955497454,48.981566671,49.007646056,49.033735613,49.059835343,49.085945250,49.112065335,49.138195602,49.164336052,49.190486688,49.216647512,49.242818527,49.268999735,49.295191139,49.321392741,49.347604543,49.373826549,
		49.400058760,49.426301178,49.452553807,49.478816649,49.505089706,49.531372980,49.557666474,49.583970191,49.610284132,49.636608301,49.662942700,49.689287331,49.715642196,49.742007298,49.768382640,49.794768224,49.821164052,49.847570127,49.873986451,49.900413027,
		49.926849856,49.953296943,49.979754288,50.006221895,50.032699765,50.059187902,50.085686308,50.112194984,50.138713935,50.165243161,50.191782666,50.218332452,50.244892521,50.271462876,50.298043519,50.324634452,50.351235679,50.377847202,50.404469022,50.431101143,
		50.457743567,50.484396296,50.511059332,50.537732679,50.564416339,50.591110314,50.617814606,50.644529218,50.671254153,50.697989412,50.724734999,50.751490915,50.778257164,50.805033747,50.831820668,50.858617928,50.885425529,50.912243476,50.939071769,50.965910411,
		50.992759405,51.019618753,51.046488458,51.073368522,51.100258948,51.127159737,51.154070893,51.180992417,51.207924313,51.234866583,51.261819229,51.288782254,51.315755659,51.342739448,51.369733624,51.396738187,51.423753142,51.450778489,51.477814233,51.504860375,
		51.531916917,51.558983863,51.586061214,51.613148973,51.640247143,51.667355725,51.694474723,51.721604139,51.748743975,51.775894234,51.803054918,51.830226029,51.857407571,51.884599545,51.911801954,51.939014801,51.966238087,51.993471816,52.020715989,52.047970610,
		52.075235681,52.102511203,52.129797180,52.157093615,52.184400508,52.211717864,52.239045684,52.266383971,52.293732727,52.321091955,52.348461658,52.375841837,52.403232495,52.430633635,52.458045258,52.485467368,52.512899968,52.540343058,52.567796643,52.595260724,
		52.622735303,52.650220384,52.677715969,52.705222059,52.732738659,52.760265769,52.787803393,52.815351533,52.842910191,52.870479370,52.898059073,52.925649301,52.953250058,52.980861345,53.008483166,53.036115522,53.063758417,53.091411852,53.119075830,53.146750353,
		53.174435424,53.202131046,53.229837221,53.257553951,53.285281239,53.313019087,53.340767498,53.368526474,53.396296018,53.424076132,53.451866818,53.479668079,53.507479918,53.535302337,53.563135338,53.590978925,53.618833098,53.646697861,53.674573217,53.702459167,
		53.730355715,53.758262862,53.786180612,53.814108966,53.842047927,53.869997497,53.897957680,53.925928477,53.953909891,53.981901924,54.009904579,54.037917859,54.065941765,54.093976301,54.122021468,54.150077270,54.178143708,54.206220785,54.234308504,54.262406867,
		54.290515876,54.318635535,54.346765845,54.374906809,54.403058429,54.431220708,54.459393649,54.487577253,54.515771524,54.543976463,54.572192074,54.600418358,54.628655319,54.656902958,54.685161278,54.713430282,54.741709972,54.770000350,54.798301419,54.826613182,
		54.854935641,54.883268798,54.911612656,54.939967218,54.968332485,54.996708461,55.025095147,55.053492547,55.081900662,55.110319495,55.138749050,55.167189327,55.195640330,55.224102061,55.252574523,55.281057717,55.309551647,55.338056316,55.366571724,55.395097876,
		55.423634773,55.452182418,55.480740813,55.509309961,55.537889865,55.566480526,55.595081947,55.623694132,55.652317081,55.680950798,55.709595286,55.738250546,55.766916581,55.795593394,55.824280987,55.852979363,55.881688523,55.910408471,55.939139210,55.967880740,
		55.996633066,56.025396189,56.054170112,56.082954838,56.111750368,56.140556706,56.169373853,56.198201813,56.227040588,56.255890180,56.284750592,56.313621826,56.342503885,56.371396772,56.400300487,56.429215036,56.458140418,56.487076638,56.516023698,56.544981600,
		56.573950346,56.602929939,56.631920382,56.660921677,56.689933827,56.718956834,56.747990700,56.777035428,56.806091021,56.835157481,56.864234810,56.893323011,56.922422086,56.951532039,56.980652871,57.009784584,57.038927182,57.068080668,57.097245042,57.126420308,
		57.155606469,57.184803526,57.214011483,57.243230341,57.272460104,57.301700773,57.330952352,57.360214843,57.389488247,57.418772569,57.448067810,57.477373972,57.506691059,57.536019072,57.565358014,57.594707888,57.624068697,57.653440442,57.682823126,57.712216751,
		57.741621321,57.771036838,57.800463303,57.829900720,57.859349091,57.888808419,57.918278706,57.947759955,57.977252167,58.006755346,58.036269494,58.065794614,58.095330707,58.124877778,58.154435827,58.184004858,58.213584872,58.243175873,58.272777864,58.302390845,
		58.332014821,58.361649793,58.391295764,58.420952737,58.450620714,58.480299697,58.509989689,58.539690692,58.569402710,58.599125744,58.628859796,58.658604871,58.688360969,58.718128094,58.747906247,58.777695432,58.807495651,58.837306907,58.867129201,58.896962537,
		58.926806917,58.956662343,58.986528818,59.016406345,59.046294925,59.076194562,59.106105258,59.136027015,59.165959836,59.195903724,59.225858680,59.255824708,59.285801810,59.315789988,59.345789246,59.375799584,59.405821007,59.435853516,59.465897114,59.495951804,
		59.526017587,59.556094467,59.586182446,59.616281526,59.646391710,59.676513001,59.706645401,59.736788912,59.766943537,59.797109279,59.827286139,59.857474121,59.887673228,59.917883460,59.948104822,59.978337315,60.008580942,60.038835706,60.069101609,60.099378653,
		60.129666841,60.159966176,60.190276660,60.220598296,60.250931085,60.281275031,60.311630137,60.341996403,60.372373834,60.402762432,60.433162199,60.463573137,60.493995250,60.524428539,60.554873007,60.585328657,60.615795491,60.646273512,60.676762722,60.707263124,
		60.737774720,60.768297512,60.798831504,60.829376698,60.859933096,60.890500700,60.921079514,60.951669540,60.982270780,61.012883237,61.043506914,61.074141812,61.104787934,61.135445284,61.166113862,61.196793673,61.227484718,61.258187000,61.288900521,61.319625285,
		61.350361292,61.381108547,61.411867052,61.442636808,61.473417819,61.504210087,61.535013614,61.565828404,61.596654458,61.627491779,61.658340370,61.689200233,61.720071370,61.750953785,61.781847480,61.812752456,61.843668718,61.874596266,61.905535105,61.936485235,
		61.967446661,61.998419384,62.029403406,62.060398731,62.091405361,62.122423298,62.153452545,62.184493105,62.215544979,62.246608171,62.277682683,62.308768518,62.339865677,62.370974164,62.402093981,62.433225131,62.464367615,62.495521438,62.526686600,62.557863106,
		62.589050956,62.620250154,62.651460702,62.682682603,62.713915860,62.745160474,62.776416449,62.807683786,62.838962489,62.870252559,62.901554000,62.932866814,62.964191004,62.995526571,63.026873519,63.058231850,63.089601567,63.120982672,63.152375167,63.183779055,
		63.215194339,63.246621022,63.278059105,63.309508591,63.340969483,63.372441784,63.403925495,63.435420619,63.466927160,63.498445118,63.529974498,63.561515301,63.593067531,63.624631188,63.656206277,63.687792800,63.719390758,63.751000155,63.782620993,63.814253275,
		63.845897003,63.877552180,63.909218809,63.940896891,63.972586429,64.004287426,64.035999885,64.067723807,64.099459196,64.131206054,64.162964384,64.194734187,64.226515468,64.258308227,64.290112468,64.321928193,64.353755405,64.385594106,64.417444299,64.449305986,
		64.481179170,64.513063853,64.544960039,64.576867728,64.608786925,64.640717631,64.672659850,64.704613583,64.736578833,64.768555602,64.800543894,64.832543711,64.864555055,64.896577928,64.928612334,64.960658275,64.992715754,65.024784772,65.056865332,65.088957438,
		65.121061091,65.153176295,65.185303051,65.217441362,65.249591231,65.281752660,65.313925651,65.346110209,65.378306334,65.410514029,65.442733297,65.474964141,65.507206563,65.539460565,65.571726151,65.604003322,65.636292081,65.668592431,65.700904374,65.733227913,
		65.765563050,65.797909788,65.830268130,65.862638077,65.895019634,65.927412801,65.959817581,65.992233978,66.024661993,66.057101630,66.089552890,66.122015777,66.154490292,66.186976439,66.219474220,66.251983637,66.284504693,66.317037391,66.349581733,66.382137722,
		66.414705359,66.447284649,66.479875592,66.512478193,66.545092453,66.577718375,66.610355961,66.643005214,66.675666137,66.708338732,66.741023001,66.773718948,66.806426574,66.839145883,66.871876876,66.904619557,66.937373928,66.970139991,67.002917750,67.035707205,
		67.068508361,67.101321220,67.134145784,67.166982055,67.199830037,67.232689732,67.265561142,67.298444270,67.331339119,67.364245690,67.397163987,67.430094013,67.463035769,67.495989258,67.528954483,67.561931447,67.594920151,67.627920599,67.660932793,67.693956735,
		67.726992429,67.760039876,67.793099080,67.826170042,67.859252765,67.892347253,67.925453506,67.958571529,67.991701324,68.024842892,68.057996237,68.091161361,68.124338267,68.157526958,68.190727435,68.223939701,68.257163760,68.290399613,68.323647263,68.356906713,
		68.390177965,68.423461022,68.456755886,68.490062560,68.523381046,68.556711347,68.590053466,68.623407405,68.656773166,68.690150753,68.723540168,68.756941413,68.790354490,68.823779403,68.857216154,68.890664746,68.924125181,68.957597461,68.991081590,69.024577569,
		69.058085402,69.091605091,69.125136638,69.158680046,69.192235318,69.225802456,69.259381462,69.292972340,69.326575092,69.360189720,69.393816227,69.427454615,69.461104888,69.494767047,69.528441096,69.562127036,69.595824870,69.629534602,69.663256233,69.696989765,
		69.730735203,69.764492547,69.798261802,69.832042968,69.865836049,69.899641048,69.933457967,69.967286808,70.001127574,70.034980268,70.068844892,70.102721449,70.136609941,70.170510372,70.204422742,70.238347056,70.272283315,70.306231523,70.340191681,70.374163792,
		70.408147860,70.442143885,70.476151872,70.510171823,70.544203739,70.578247624,70.612303481,70.646371311,70.680451118,70.714542904,70.748646671,70.782762422,70.816890161,70.851029888,70.885181607,70.919345321,70.953521031,70.987708741,71.021908454,71.056120171,
		71.090343895,71.124579629,71.158827375,71.193087136,71.227358915,71.261642714,71.295938536,71.330246383,71.364566257,71.398898162,71.433242100,71.467598074,71.501966086,71.536346138,71.570738234,71.605142375,71.639558565,71.673986806,71.708427100,71.742879451,
		71.777343860,71.811820330,71.846308865,71.880809465,71.915322135,71.949846877,71.984383692,72.018932584,72.053493556,72.088066609,72.122651747,72.157248972,72.191858286,72.226479693,72.261113194,72.295758793,72.330416492,72.365086293,72.399768199,72.434462213,
		72.469168337,72.503886574,72.538616926,72.573359396,72.608113987,72.642880701,72.677659541,72.712450509,72.747253607,72.782068839,72.816896208,72.851735714,72.886587362,72.921451154,72.956327092,72.991215179,73.026115417,73.061027810,73.095952359,73.130889067,
		73.165837937,73.200798972,73.235772174,73.270757545,73.305755088,73.340764806,73.375786702,73.410820777,73.445867035,73.480925478,73.515996108,73.551078929,73.586173942,73.621281151,73.656400558,73.691532166,73.726675976,73.761831993,73.797000217,73.832180653,
		73.867373302,73.902578167,73.937795251,73.973024556,74.008266085,74.043519841,74.078785825,74.114064041,74.149354491,74.184657178,74.219972105,74.255299273,74.290638686,74.325990345,74.361354255,74.396730417,74.432118834,74.467519508,74.502932442,74.538357639,
		74.573795102,74.609244832,74.644706832,74.680181106,74.715667655,74.751166482,74.786677590,74.822200982,74.857736659,74.893284625,74.928844882,74.964417433,75.000002280,75.035599426,75.071208874,75.106830626,75.142464684,75.178111052,75.213769731,75.249440725,
		75.285124036,75.320819667,75.356527620,75.392247898,75.427980503,75.463725438,75.499482706,75.535252309,75.571034250,75.606828532,75.642635156,75.678454126,75.714285444,75.750129113,75.785985135,75.821853513,75.857734250,75.893627348,75.929532810,75.965450638,
		76.001380835,76.037323404,76.073278347,76.109245666,76.145225365,76.181217446,76.217221911,76.253238764,76.289268006,76.325309641,76.361363670,76.397430097,76.433508924,76.469600154,76.505703789,76.541819832,76.577948286,76.614089153,76.650242435,76.686408136,
		76.722586258,76.758776803,76.794979775,76.831195175,76.867423006,76.903663272,76.939915974,76.976181115,77.012458697,77.048748724,77.085051198,77.121366122,77.157693498,77.194033328,77.230385616,77.266750364,77.303127574,77.339517250,77.375919393,77.412334006,
		77.448761093,77.485200655,77.521652695,77.558117216,77.594594221,77.631083711,77.667585690,77.704100160,77.740627124,77.777166585,77.813718544,77.850283005,77.886859971,77.923449443,77.960051424,77.996665918,78.033292926,78.069932452,78.106584497,78.143249065,
		78.179926158,78.216615778,78.253317929,78.290032613,78.326759832,78.363499590,78.400251888,78.437016730,78.473794117,78.510584053,78.547386540,78.584201581,78.621029178,78.657869335,78.694722053,78.731587335,78.768465184,78.805355602,78.842258593,78.879174158,
		78.916102300,78.953043022,78.989996327,79.026962217,79.063940694,79.100931762,79.137935423,79.174951679,79.211980534,79.249021989,79.286076048,79.323142712,79.360221986,79.397313870,79.434418369,79.471535483,79.508665217,79.545807573,79.582962553,79.620130160,
		79.657310396,79.694503264,79.731708768,79.768926908,79.806157689,79.843401112,79.880657180,79.917925897,79.955207263,79.992501283,80.029807958,80.067127292,80.104459287,80.141803945,80.179161269,80.216531263,80.253913927,80.291309266,80.328717281,80.366137975,
		80.403571351,80.441017412,80.478476160,80.515947597,80.553431727,80.590928552,80.628438075,80.665960297,80.703495223,80.741042854,80.778603192,80.816176242,80.853762005,80.891360484,80.928971681,80.966595600,81.004232242,81.041881611,81.079543708,81.117218538,
		81.154906101,81.192606402,81.230319442,81.268045224,81.305783751,81.343535025,81.381299050,81.419075827,81.456865359,81.494667649,81.532482699,81.570310513,81.608151092,81.646004440,81.683870559,81.721749452,81.759641120,81.797545568,81.835462797,81.873392810,
		81.911335610,81.949291199,81.987259580,82.025240756,82.063234729,82.101241503,82.139261078,82.177293459,82.215338647,82.253396646,82.291467458,82.329551085,82.367647531,82.405756798,82.443878888,82.482013804,82.520161549,82.558322126,82.596495536,82.634681784,
		82.672880870,82.711092799,82.749317572,82.787555192,82.825805662,82.864068985,82.902345162,82.940634197,82.978936093,83.017250852,83.055578476,83.093918968,83.132272332,83.170638568,83.209017681,83.247409673,83.285814546,83.324232303,83.362662947,83.401106480,
		83.439562905,83.478032224,83.516514441,83.555009558,83.593517577,83.632038501,83.670572333,83.709119075,83.747678730,83.786251301,83.824836790,83.863435200,83.902046534,83.940670793,83.979307982,84.017958102,84.056621156,84.095297146,84.133986076,84.172687949,
		84.211402765,84.250130529,84.288871243,84.327624909,84.366391531,84.405171110,84.443963650,84.482769153,84.521587622,84.560419059,84.599263466,84.638120848,84.676991206,84.715874543,84.754770861,84.793680164,84.832602453,84.871537732,84.910486003,84.949447268,
		84.988421532,85.027408795,85.066409060,85.105422332,85.144448611,85.183487900,85.222540203,85.261605522,85.300683859,85.339775218,85.378879600,85.417997009,85.457127447,85.496270916,85.535427420,85.574596961,85.613779541,85.652975164,85.692183832,85.731405548,
		85.770640313,85.809888132,85.849149006,85.888422938,85.927709931,85.967009988,86.006323110,86.045649302,86.084988564,86.124340901,86.163706314,86.203084807,86.242476382,86.281881041,86.321298788,86.360729625,86.400173554,86.439630578,86.479100700,86.518583923,
		86.558080249,86.597589680,86.637112220,86.676647872,86.716196636,86.755758518,86.795333518,86.834921640,86.874522886,86.914137260,86.953764763,86.993405398,87.033059168,87.072726076,87.112406124,87.152099315,87.191805651,87.231525136,87.271257771,87.311003560,
		87.350762505,87.390534609,87.430319874,87.470118303,87.509929899,87.549754665,87.589592602,87.629443715,87.669308004,87.709185474,87.749076126,87.788979964,87.828896989,87.868827205,87.908770615,87.948727220,87.988697024,88.028680029,88.068676238,88.108685653,
		88.148708278,88.188744115,88.228793166,88.268855434,88.308930922,88.349019633,88.389121568,88.429236732,88.469365126,88.509506753,88.549661615,88.589829716,88.630011059,88.670205644,88.710413477,88.750634558,88.790868891,88.831116478,88.871377323,88.911651427,
		88.951938793,88.992239425,89.032553323,89.072880493,89.113220935,89.153574653,89.193941649,89.234321926,89.274715487,89.315122334,89.355542469,89.395975897,89.436422619,89.476882637,89.517355955,89.557842575,89.598342501,89.638855733,89.679382276,89.719922132,
		89.760475304,89.801041793,89.841621603,89.882214737,89.922821197,89.963440986,90.004074107,90.044720561,90.085380353,90.126053484,90.166739957,90.207439774,90.248152940,90.288879455,90.329619324,90.370372547,90.411139129,90.451919072,90.492712378,90.533519050,
		90.574339092,90.615172504,90.656019291,90.696879454,90.737752997,90.778639922,90.819540232,90.860453930,90.901381017,90.942321498,90.983275373,91.024242647,91.065223322,91.106217400,91.147224884,91.188245777,91.229280082,91.270327801,91.311388936,91.352463491,
		91.393551468,91.434652870,91.475767700,91.516895959,91.558037652,91.599192780,91.640361346,91.681543353,91.722738803,91.763947700,91.805170045,91.846405842,91.887655093,91.928917801,91.970193969,92.011483598,92.052786693,92.094103255,92.135433287,92.176776793,
		92.218133774,92.259504233,92.300888173,92.342285596,92.383696506,92.425120905,92.466558795,92.508010180,92.549475061,92.590953442,92.632445326,92.673950714,92.715469610,92.757002017,92.798547936,92.840107371,92.881680325,92.923266799,92.964866797,93.006480322,
		93.048107375,93.089747961,93.131402081,93.173069737,93.214750934,93.256445673,93.298153957,93.339875789,93.381611171,93.423360107,93.465122598,93.506898648,93.548688259,93.590491434,93.632308175,93.674138486,93.715982368,93.757839825,93.799710860,93.841595474,
		93.883493671,93.925405453,93.967330823,94.009269784,94.051222338,94.093188488,94.135168237,94.177161587,94.219168542,94.261189103,94.303223273,94.345271056,94.387332453,94.429407468,94.471496103,94.513598360,94.555714244,94.597843755,94.639986897,94.682143673,
		94.724314085,94.766498136,94.808695829,94.850907166,94.893132150,94.935370784,94.977623070,95.019889011,95.062168609,95.104461868,95.146768791,95.189089378,95.231423635,95.273771562,95.316133164,95.358508442,95.400897399,95.443300038,95.485716361,95.528146372,
		95.570590073,95.613047466,95.655518555,95.698003342,95.740501829,95.783014020,95.825539917,95.868079522,95.910632839,95.953199870,95.995780618,96.038375086,96.080983276,96.123605190,96.166240832,96.208890205,96.251553310,96.294230151,96.336920731,96.379625051,
		96.422343115,96.465074926,96.507820486,96.550579797,96.593352863,96.636139686,96.678940269,96.721754615,96.764582725,96.807424604,96.850280253,96.893149676,96.936032875,96.978929852,97.021840611,97.064765154,97.107703484,97.150655603,97.193621514,97.236601220,
		97.279594724,97.322602028,97.365623135,97.408658048,97.451706769,97.494769301,97.537845646,97.580935808,97.624039790,97.667157593,97.710289220,97.753434675,97.796593960,97.839767077,97.882954030,97.926154820,97.969369452,98.012597926,98.055840247,98.099096417,
		98.142366438,98.185650313,98.228948045,98.272259637,98.315585091,98.358924410,98.402277596,98.445644653,98.489025583,98.532420389,98.575829073,98.619251638,98.662688087,98.706138423,98.749602648,98.793080765,98.836572777,98.880078686,98.923598495,98.967132207,
		99.010679824,99.054241350,99.097816786,99.141406136,99.185009402,99.228626587,99.272257694,99.315902725,99.359561684,99.403234572,99.446921392,99.490622148,99.534336842,99.578065476,99.621808054,99.665564577,99.709335049,99.753119473,99.796917851,99.840730185,
		99.884556479,99.928396736,99.972250957,100.016119146,100.060001305,100.103897437,100.147807545,100.191731632,100.235669699,100.279621751,100.323587789,100.367567816,100.411561836,100.455569850,100.499591861,100.543627873,100.587677887,100.631741907,100.675819935,100.719911974,
		100.764018027,100.808138096,100.852272184,100.896420294,100.940582429,100.984758590,101.028948782,101.073153006,101.117371265,101.161603563,101.205849901,101.250110282,101.294384710,101.338673187,101.382975715,101.427292298,101.471622937,101.515967636,101.560326398,101.604699225,
		101.649086120,101.693487085,101.737902123,101.782331238,101.826774431,101.871231705,101.915703064,101.960188510,102.004688045,102.049201672,102.093729394,102.138271214,102.182827135,102.227397158,102.271981287,102.316579525,102.361191874,102.405818337,102.450458917,102.495113616,
		102.539782438,102.584465384,102.629162457,102.673873661,102.718598998,102.763338471,102.808092082,102.852859834,102.897641730,102.942437772,102.987247964,103.032072308,103.076910806,103.121763462,103.166630278,103.211511257,103.256406402,103.301315714,103.346239198,103.391176856,
		103.436128690,103.481094703,103.526074898,103.571069278,103.616077845,103.661100602,103.706137552,103.751188697,103.796254040,103.841333585,103.886427333,103.931535287,103.976657451,104.021793826,104.066944416,104.112109223,104.157288250,104.202481500,104.247688975,104.292910679,
		104.338146613,104.383396781,104.428661185,104.473939828,104.519232713,104.564539842,104.609861219,104.655196845,104.700546725,104.745910859,104.791289252,104.836681905,104.882088822,104.927510005,104.972945457,105.018395181,105.063859180,105.109337455,105.154830011,105.200336849,
		105.245857972,105.291393383,105.336943086,105.382507081,105.428085373,105.473677964,105.519284857,105.564906054,105.610541558,105.656191372,105.701855499,105.747533941,105.793226701,105.838933782,105.884655187,105.930390917,105.976140977,106.021905368,106.067684094,106.113477157,
		106.159284560,106.205106305,106.250942396,106.296792835,106.342657625,106.388536768,106.434430267,106.480338126,106.526260346,106.572196931,106.618147883,106.664113204,106.710092899,106.756086969,106.802095416,106.848118245,106.894155458,106.940207056,106.986273044,107.032353423,
		107.078448197,107.124557369,107.170680940,107.216818914,107.262971293,107.309138081,107.355319279,107.401514891,107.447724920,107.493949367,107.540188237,107.586441531,107.632709252,107.678991403,107.725287987,107.771599007,107.817924465,107.864264364,107.910618706,107.956987495,
		108.003370734,108.049768424,108.096180569,108.142607171,108.189048234,108.235503759,108.281973750,108.328458209,108.374957139,108.421470544,108.467998424,108.514540784,108.561097627,108.607668953,108.654254768,108.700855073,108.747469870,108.794099164,108.840742956,108.887401249,
		108.934074046,108.980761349,109.027463163,109.074179488,109.120910328,109.167655686,109.214415565,109.261189966,109.307978893,109.354782349,109.401600337,109.448432858,109.495279916,109.542141514,109.589017654,109.635908340,109.682813573,109.729733356,109.776667693,109.823616586,
		109.870580038,109.917558051,109.964550629,110.011557773,110.058579488,110.105615774,110.152666637,110.199732077,110.246812097,110.293906702,110.341015892,110.388139672,110.435278043,110.482431008,110.529598571,110.576780734,110.623977499,110.671188870,110.718414849,110.765655440,
		110.812910643,110.860180463,110.907464903,110.954763964,111.002077650,111.049405963,111.096748907,111.144106483,111.191478695,111.238865546,111.286267037,111.333683173,111.381113955,111.428559387,111.476019470,111.523494209,111.570983605,111.618487662,111.666006382,111.713539768,
		111.761087822,111.808650548,111.856227948,111.903820025,111.951426781,111.999048220,112.046684344,112.094335156,112.142000659,112.189680855,112.237375747,112.285085338,112.332809631,112.380548628,112.428302332,112.476070746,112.523853873,112.571651716,112.619464276,112.667291558,
		112.715133563,112.762990294,112.810861755,112.858747948,112.906648875,112.954564540,113.002494945,113.050440093,113.098399987,113.146374629,113.194364023,113.242368170,113.290387074,113.338420738,113.386469164,113.434532355,113.482610314,113.530703043,113.578810545,113.626932824,
		113.675069881,113.723221720,113.771388343,113.819569754,113.867765954,113.915976946,113.964202734,114.012443321,114.060698707,114.108968898,114.157253895,114.205553701,114.253868319,114.302197752,114.350542002,114.398901072,114.447274965,114.495663684,114.544067231,114.592485609,
		114.640918822,114.689366871,114.737829759,114.786307490,114.834800066,114.883307490,114.931829764,114.980366891,115.028918875,115.077485717,115.126067421,115.174663989,115.223275425,115.271901730,115.320542908,115.369198961,115.417869893,115.466555705,115.515256401,115.563971983,
		115.612702455,115.661447819,115.710208077,115.758983233,115.807773289,115.856578248,115.905398113,115.954232887,116.003082572,116.051947171,116.100826687,116.149721122,116.198630480,116.247554763,116.296493974,116.345448115,116.394417190,116.443401202,116.492400152,116.541414044,
		116.590442881,116.639486665,116.688545398,116.737619085,116.786707728,116.835811328,116.884929890,116.934063416,116.983211908,117.032375370,117.081553804,117.130747213,117.179955600,117.229178967,117.278417318,117.327670654,117.376938980,117.426222297,117.475520608,117.524833917,
		117.574162225,117.623505537,117.672863853,117.722237178,117.771625514,117.821028863,117.870447229,117.919880615,117.969329022,118.018792455,118.068270915,118.117764405,118.167272928,118.216796488,118.266335086,118.315888726,118.365457410,118.415041141,118.464639921,118.514253755,
		118.563882644,118.613526590,118.663185598,118.712859670,118.762548808,118.812253015,118.861972294,118.911706648,118.961456079,119.011220591,119.061000186,119.110794867,119.160604636,119.210429497,119.260269452,119.310124504,119.359994656,119.409879911,119.459780270,119.509695738,
		119.559626317,119.609572009,119.659532818,119.709508746,119.759499796,119.809505971,119.859527273,119.909563706,119.959615272,120.009681974,120.059763814,120.109860796,120.159972922,120.210100195,120.260242617,120.310400193,120.360572923,120.410760812,120.460963862,120.511182075,
		120.561415455,120.611664004,120.661927726,120.712206622,120.762500695,120.812809949,120.863134386,120.913474009,120.963828821,121.014198824,121.064584022,121.114984416,121.165400011,121.215830808,121.266276810,121.316738021,121.367214443,121.417706078,121.468212930,121.518735002,
		121.569272295,121.619824814,121.670392560,121.720975536,121.771573746,121.822187192,121.872815877,121.923459803,121.974118974,122.024793392,122.075483060,122.126187981,122.176908157,122.227643591,122.278394287,122.329160247,122.379941473,122.430737969,122.481549737,122.532376780,
		122.583219101,122.634076703,122.684949588,122.735837759,122.786741219,122.837659972,122.888594018,122.939543362,122.990508006,123.041487954,123.092483207,123.143493768,123.194519641,123.245560828,123.296617331,123.347689155,123.398776301,123.449878772,123.500996571,123.552129701,
		123.603278165,123.654441965,123.705621105,123.756815586,123.808025412,123.859250586,123.910491110,123.961746988,124.013018221,124.064304813,124.115606767,124.166924085,124.218256771,124.269604826,124.320968254,124.372347057,124.423741239,124.475150802,124.526575749,124.578016083,
		124.629471806,124.680942921,124.732429432,124.783931340,124.835448649,124.886981362,124.938529481,124.990093008,125.041671948,125.093266303,125.144876075,125.196501267,125.248141882,125.299797923,125.351469393,125.403156294,125.454858630,125.506576402,125.558309615,125.610058270,
		125.661822370,125.713601919,125.765396918,125.817207372,125.869033282,125.920874652,125.972731484,126.024603780,126.076491545,126.128394780,126.180313489,126.232247674,126.284197338,126.336162484,126.388143115,126.440139233,126.492150841,126.544177942,126.596220539,126.648278635,
		126.700352232,126.752441333,126.804545941,126.856666060,126.908801690,126.960952837,127.013119501,127.065301687,127.117499397,127.169712633,127.221941398,127.274185696,127.326445529,127.378720900,127.431011811,127.483318266,127.535640268,127.587977818,127.640330920,127.692699577,
		127.745083792,127.797483567,127.849898904,127.902329808,127.954776281,128.007238325,128.059715943,128.112209139,128.164717914,128.217242273,128.269782217,128.322337749,128.374908872,128.427495590,128.480097904,128.532715818,128.585349334,128.637998455,128.690663185,128.743343525,
		128.796039479,128.848751049,128.901478239,128.954221050,129.006979487,129.059753551,129.112543246,129.165348574,129.218169538,129.271006141,129.323858386,129.376726276,129.429609812,129.482508999,129.535423839,129.588354335,129.641300489,129.694262305,129.747239785,129.800232931,
		129.853241748,129.906266238,129.959306403,130.012362246,130.065433770,130.118520978,130.171623873,130.224742458,130.277876734,130.331026706,130.384192376,130.437373747,130.490570821,130.543783602,130.597012092,130.650256294,130.703516211,130.756791845,130.810083200,130.863390278,
		130.916713083,130.970051616,131.023405881,131.076775881,131.130161618,131.183563095,131.236980316,131.290413282,131.343861996,131.397326462,131.450806683,131.504302660,131.557814398,131.611341898,131.664885164,131.718444198,131.772019003,131.825609583,131.879215939,131.932838074,
		131.986475993,132.040129696,132.093799188,132.147484470,132.201185546,132.254902419,132.308635091,132.362383566,132.416147845,132.469927932,132.523723830,132.577535541,132.631363069,132.685206416,132.739065584,132.792940578,132.846831399,132.900738050,132.954660535,133.008598855,
		133.062553015,133.116523016,133.170508861,133.224510555,133.278528098,133.332561494,133.386610746,133.440675856,133.494756828,133.548853665,133.602966368,133.657094941,133.711239387,133.765399709,133.819575909,133.873767990,133.927975955,133.982199807,134.036439549,134.090695183,
		134.144966713,134.199254141,134.253557469,134.307876702,134.362211841,134.416562890,134.470929851,134.525312727,134.579711521,134.634126236,134.688556874,134.743003439,134.797465933,134.851944360,134.906438721,134.960949019,135.015475259,135.070017442,135.124575571,135.179149649,
		135.233739679,135.288345663,135.342967606,135.397605508,135.452259374,135.506929206,135.561615006,135.616316779,135.671034525,135.725768250,135.780517954,135.835283641,135.890065315,135.944862976,135.999676630,136.054506278,136.109351922,136.164213567,136.219091215,136.273984868,
		136.328894530,136.383820203,136.438761891,136.493719595,136.548693319,136.603683066,136.658688838,136.713710639,136.768748470,136.823802336,136.878872239,136.933958181,136.989060165,137.044178195,137.099312274,137.154462403,137.209628586,137.264810825,137.320009124,137.375223486,
		137.430453913,137.485700407,137.540962973,137.596241612,137.651536327,137.706847122,137.762174000,137.817516962,137.872876012,137.928251152,137.983642387,138.039049717,138.094473147,138.149912679,138.205368316,138.260840060,138.316327915,138.371831884,138.427351968,138.482888172,
		138.538440498,138.594008948,138.649593526,138.705194235,138.760811076,138.816444054,138.872093171,138.927758429,138.983439833,139.039137383,139.094851084,139.150580938,139.206326948,139.262089117,139.317867448,139.373661943,139.429472605,139.485299438,139.541142443,139.597001625,
		139.652876985,139.708768527,139.764676253,139.820600167,139.876540270,139.932496567,139.988469059,140.044457750,140.100462643,140.156483739,140.212521043,140.268574557,140.324644284,140.380730226,140.436832387,140.492950769,140.549085376,140.605236209,140.661403273,140.717586569,
		140.773786101,140.830001871,140.886233883,140.942482138,140.998746641,141.055027393,141.111324399,141.167637659,141.223967178,141.280312959,141.336675003,141.393053314,141.449447896,141.505858749,141.562285878,141.618729286,141.675188974,141.731664947,141.788157206,141.844665755,
		141.901190597,141.957731734,142.014289169,142.070862906,142.127452946,142.184059293,142.240681950,142.297320919,142.353976204,142.410647807,142.467335731,142.524039978,142.580760553,142.637497457,142.694250693,142.751020265,142.807806175,142.864608426,142.921427021,142.978261963,
		143.035113254,143.091980897,143.148864896,143.205765253,143.262681971,143.319615053,143.376564501,143.433530319,143.490512509,143.547511075,143.604526018,143.661557343,143.718605051,143.775669146,143.832749631,143.889846508,143.946959780,144.004089450,144.061235521,144.118397996,
		144.175576877,144.232772169,144.289983872,144.347211990,144.404456527,144.461717485,144.518994866,144.576288674,144.633598911,144.690925581,144.748268686,144.805628229,144.863004213,144.920396641,144.977805515,145.035230839,145.092672616,145.150130847,145.207605537,145.265096687,
		145.322604301,145.380128382,145.437668933,145.495225955,145.552799453,145.610389429,145.667995886,145.725618827,145.783258255,145.840914172,145.898586581,145.956275486,146.013980888,146.071702792,146.129441199,146.187196114,146.244967537,146.302755474,146.360559925,146.418380895,
		146.476218385,146.534072400,146.591942941,146.649830011,146.707733615,146.765653753,146.823590430,146.881543648,146.939513409,146.997499718,147.055502576,147.113521986,147.171557952,147.229610476,147.287679561,147.345765210,147.403867426,147.461986212,147.520121570,147.578273504,
		147.636442016,147.694627109,147.752828786,147.811047050,147.869281903,147.927533350,147.985801391,148.044086031,148.102387273,148.160705118,148.219039570,148.277390632,148.335758307,148.394142598,148.452543506,148.510961036,148.569395190,148.627845972,148.686313383,148.744797427,
		148.803298106,148.861815424,148.920349383,148.978899987,149.037467238,149.096051138,149.154651692,149.213268901,149.271902769,149.330553298,149.389220492,149.447904352,149.506604883,149.565322087,149.624055966,149.682806525,149.741573764,149.800357689,149.859158300,149.917975602,
		149.976809597,150.035660287,150.094527677,150.153411768,150.212312563,150.271230066,150.330164279,150.389115206,150.448082848,150.507067210,150.566068293,150.625086100,150.684120636,150.743171901,150.802239900,150.861324635,150.920426109,150.979544325,151.038679286,151.097830994,
		151.156999453,151.216184666,151.275386634,151.334605362,151.393840852,151.453093106,151.512362129,151.571647922,151.630950488,151.690269831,151.749605954,151.808958858,151.868328547,151.927715025,151.987118293,152.046538355,152.105975213,152.165428871,152.224899331,152.284386596,
		152.343890670,152.403411554,152.462949252,152.522503767,152.582075101,152.641663258,152.701268241,152.760890051,152.820528693,152.880184168,152.939856480,152.999545633,153.059251627,153.118974468,153.178714156,153.238470696,153.298244090,153.358034341,153.417841452,153.477665426,
		153.537506265,153.597363973,153.657238553,153.717130007,153.777038338,153.836963549,153.896905643,153.956864623,154.016840491,154.076833252,154.136842906,154.196869459,154.256912911,154.316973267,154.377050528,154.437144699,154.497255781,154.557383778,154.617528693,154.677690528,
		154.737869287,154.798064971,154.858277585,154.918507131,154.978753612,155.039017030,155.099297389,155.159594692,155.219908941,155.280240140,155.340588291,155.400953397,155.461335460,155.521734485,155.582150474,155.642583429,155.703033354,155.763500251,155.823984123,155.884484974,
		155.945002806,156.005537621,156.066089424,156.126658216,156.187244002,156.247846782,156.308466561,156.369103342,156.429757126,156.490427918,156.551115719,156.611820534,156.672542364,156.733281213,156.794037084,156.854809979,156.915599901,156.976406854,157.037230840,157.098071861,
		157.158929922,157.219805024,157.280697172,157.341606366,157.402532612,157.463475910,157.524436265,157.585413679,157.646408155,157.707419696,157.768448305,157.829493984,157.890556738,157.951636567,158.012733476,158.073847468,158.134978545,158.196126709,158.257291965,158.318474315,
		158.379673761,158.440890307,158.502123956,158.563374710,158.624642573,158.685927546,158.747229634,158.808548839,158.869885164,158.931238612,158.992609186,159.053996888,159.115401722,159.176823690,159.238262795,159.299719041,159.361192430,159.422682965,159.484190649,159.545715484,
		159.607257475,159.668816623,159.730392932,159.791986404,159.853597042,159.915224849,159.976869829,160.038531984,160.100211316,160.161907830,160.223621527,160.285352411,160.347100484,160.408865750,160.470648211,160.532447870,160.594264731,160.656098795,160.717950066,160.779818548,
		160.841704242,160.903607151,160.965527279,161.027464629,161.089419203,161.151391004,161.213380035,161.275386300,161.337409800,161.399450539,161.461508520,161.523583746,161.585676219,161.647785943,161.709912920,161.772057153,161.834218646,161.896397400,161.958593420,162.020806708,
		162.083037266,162.145285098,162.207550207,162.269832595,162.332132266,162.394449222,162.456783466,162.519135001,162.581503831,162.643889957,162.706293383,162.768714112,162.831152147,162.893607491,162.956080145,163.018570115,163.081077401,163.143602008,163.206143938,163.268703194,
		163.331279779,163.393873696,163.456484947,163.519113536,163.581759466,163.644422739,163.707103359,163.769801328,163.832516649,163.895249326,163.957999360,164.020766756,164.083551515,164.146353641,164.209173137,164.272010005,164.334864249,164.397735871,164.460624875,164.523531262,
		164.586455037,164.649396202,164.712354760,164.775330714,164.838324067,164.901334821,164.964362981,165.027408547,165.090471524,165.153551915,165.216649721,165.279764947,165.342897595,165.406047668,165.469215169,165.532400101,165.595602467,165.658822269,165.722059511,165.785314195,
		165.848586325,165.911875903,165.975182933,166.038507416,166.101849357,166.165208758,166.228585621,166.291979951,166.355391749,166.418821019,166.482267763,166.545731985,166.609213687,166.672712873,166.736229545,166.799763706,166.863315359,166.926884508,166.990471154,167.054075301,
		167.117696952,167.181336109,167.244992776,167.308666956,167.372358651,167.436067864,167.499794599,167.563538858,167.627300643,167.691079959,167.754876808,167.818691193,167.882523117,167.946372582,168.010239592,168.074124150,168.138026258,168.201945919,168.265883137,168.329837914,
		168.393810253,168.457800157,168.521807629,168.585832672,168.649875289,168.713935483,168.778013257,168.842108613,168.906221554,168.970352085,169.034500206,169.098665922,169.162849235,169.227050148,169.291268664,169.355504787,169.419758518,169.484029861,169.548318819,169.612625394,
		169.676949591,169.741291410,169.805650857,169.870027932,169.934422640,169.998834984,170.063264965,170.127712588,170.192177854,170.256660768,170.321161331,170.385679547,170.450215419,170.514768950,170.579340142,170.643928999,170.708535523,170.773159718,170.837801586,170.902461130,
		170.967138353,171.031833259,171.096545849,171.161276128,171.226024097,171.290789760,171.355573120,171.420374179,171.485192941,171.550029409,171.614883585,171.679755472,171.744645074,171.809552393,171.874477432,171.939420194,172.004380683,172.069358900,172.134354849,172.199368533,
		172.264399955,172.329449117,172.394516023,172.459600676,172.524703078,172.589823233,172.654961143,172.720116811,172.785290240,172.850481434,172.915690394,172.980917125,173.046161629,173.111423908,173.176703966,173.242001806,173.307317431,173.372650844,173.438002047,173.503371043,
		173.568757836,173.634162428,173.699584822,173.765025022,173.830483030,173.895958849,173.961452482,174.026963932,174.092493202,174.158040295,174.223605213,174.289187960,174.354788539,174.420406953,174.486043204,174.551697295,174.617369230,174.683059011,174.748766642,174.814492124,
		174.880235462,174.945996658,175.011775715,175.077572636,175.143387424,175.209220081,175.275070612,175.340939018,175.406825303,175.472729469,175.538651520,175.604591459,175.670549288,175.736525010,175.802518628,175.868530146,175.934559566,176.000606891,176.066672124,176.132755268,
		176.198856326,176.264975301,176.331112196,176.397267014,176.463439757,176.529630429,176.595839032,176.662065570,176.728310046,176.794572462,176.860852821,176.927151127,176.993467382,177.059801589,177.126153751,177.192523871,177.258911953,177.325317998,177.391742010,177.458183992,
		177.524643947,177.591121878,177.657617787,177.724131678,177.790663553,177.857213416,177.923781270,177.990367117,178.056970960,178.123592803,178.190232647,178.256890497,178.323566356,178.390260225,178.456972108,178.523702008,178.590449929,178.657215872,178.723999841,178.790801838,
		178.857621868,178.924459932,178.991316034,179.058190176,179.125082362,179.191992594,179.258920876,179.325867210,179.392831599,179.459814047,179.526814556,179.593833129,179.660869769,179.727924479,179.794997262,179.862088121,179.929197059,179.996324079,180.063469184,180.130632376,
		180.197813659,180.265013036,180.332230510,180.399466083,180.466719758,180.533991539,180.601281429,180.668589429,180.735915544,180.803259777,180.870622129,180.938002605,181.005401207,181.072817938,181.140252801,181.207705799,181.275176935,181.342666212,181.410173633,181.477699200,
		181.545242918,181.612804788,181.680384813,181.747982998,181.815599344,181.883233854,181.950886532,182.018557380,182.086246402,182.153953600,182.221678978,182.289422537,182.357184282,182.424964215,182.492762340,182.560578658,182.628413173,182.696265889,182.764136807,182.832025931,
		182.899933264,182.967858809,183.035802569,183.103764547,183.171744745,183.239743167,183.307759815,183.375794694,183.443847804,183.511919150,183.580008735,183.648116561,183.716242631,183.784386949,183.852549517,183.920730339,183.988929416,184.057146753,184.125382352,184.193636216,
		184.261908348,184.330198751,184.398507428,184.466834382,184.535179615,184.603543132,184.671924935,184.740325026,184.808743409,184.877180087,184.945635062,185.014108338,185.082599918,185.151109804,185.219638000,185.288184508,185.356749332,185.425332474,185.493933938,185.562553725,
		185.631191841,185.699848286,185.768523065,185.837216180,185.905927634,185.974657430,186.043405571,186.112172060,186.180956900,186.249760095,186.318581646,186.387421557,186.456279831,186.525156471,186.594051480,186.662964860,186.731896615,186.800846748,186.869815262,186.938802159,
		187.007807443,187.076831116,187.145873182,187.214933644,187.284012504,187.353109765,187.422225431,187.491359504,187.560511988,187.629682885,187.698872198,187.768079931,187.837306085,187.906550665,187.975813673,188.045095113,188.114394986,188.183713296,188.253050047,188.322405240,
		188.391778880,188.461170968,188.530581508,188.600010504,188.669457957,188.738923871,188.808408249,188.877911093,188.947432407,189.016972194,189.086530457,189.156107198,189.225702422,189.295316129,189.364948324,189.434599010,189.504268190,189.573955865,189.643662041,189.713386718,
		189.783129902,189.852891593,189.922671796,189.992470513,190.062287748,190.132123503,190.201977781,190.271850586,190.341741919,190.411651785,190.481580186,190.551527125,190.621492606,190.691476630,190.761479201,190.831500323,190.901539997,190.971598228,191.041675017,191.111770369,
		191.181884285,191.252016770,191.322167825,191.392337454,191.462525660,191.532732445,191.602957814,191.673201768,191.743464311,191.813745445,191.884045175,191.954363502,192.024700430,192.095055961,192.165430099,192.235822847,192.306234207,192.376664182,192.447112777,192.517579993,
		192.588065833,192.658570300,192.729093399,192.799635130,192.870195498,192.940774506,193.011372156,193.081988451,193.152623394,193.223276989,193.293949238,193.364640145,193.435349711,193.506077941,193.576824837,193.647590403,193.718374640,193.789177553,193.859999144,193.930839416,
		194.001698372,194.072576015,194.143472349,194.214387375,194.285321097,194.356273519,194.427244643,194.498234471,194.569243008,194.640270255,194.711316217,194.782380895,194.853464294,194.924566415,194.995687262,195.066826838,195.137985146,195.209162189,195.280357970,195.351572491,
		195.422805756,195.494057768,195.565328530,195.636618045,195.707926316,195.779253345,195.850599136,195.921963691,195.993347015,196.064749109,196.136169977,196.207609621,196.279068045,196.350545252,196.422041244,196.493556026,196.565089598,196.636641965,196.708213130,196.779803096,
		196.851411865,196.923039440,196.994685826,197.066351023,197.138035037,197.209737868,197.281459522,197.353200000,197.424959305,197.496737441,197.568534410,197.640350216,197.712184861,197.784038348,197.855910682,197.927801863,197.999711896,198.071640784,198.143588528,198.215555134,
		198.287540602,198.359544937,198.431568142,198.503610218,198.575671170,198.647751001,198.719849713,198.791967309,198.864103793,198.936259166,199.008433434,199.080626597,199.152838660,199.225069625,199.297319496,199.369588275,199.441875965,199.514182569,199.586508091,199.658852533,
		199.731215898,199.803598190,199.875999411,199.948419564,200.020858652,200.093316679,200.165793647,200.238289559,200.310804418,200.383338228,200.455890991,200.528462710,200.601053389,200.673663029,200.746291635,200.818939209,200.891605755,200.964291274,201.036995771,201.109719248,
		201.182461709,201.255223155,201.328003591,201.400803019,201.473621442,201.546458864,201.619315287,201.692190714,201.765085148,201.837998592,201.910931050,201.983882524,202.056853017,202.129842532,202.202851073,202.275878642,202.348925242,202.421990876,202.495075548,202.568179260,
		202.641302015,202.714443816,202.787604667,202.860784570,202.933983528,203.007201544,203.080438622,203.153694763,203.226969973,203.300264252,203.373577605,203.446910034,203.520261542,203.593632132,203.667021808,203.740430572,203.813858428,203.887305378,203.960771425,204.034256572,
		204.107760823,204.181284180,204.254826646,204.328388225,204.401968919,204.475568732,204.549187665,204.622825723,204.696482909,204.770159225,204.843854674,204.917569260,204.991302985,205.065055852,205.138827865,205.212619027,205.286429339,205.360258807,205.434107431,205.507975216,
		205.581862165,205.655768280,205.729693565,205.803638022,205.877601654,205.951584465,206.025586458,206.099607635,206.173647999,206.247707554,206.321786303,206.395884248,206.470001393,206.544137741,206.618293294,206.692468055,206.766662029,206.840875217,206.915107622,206.989359248,
		207.063630098,207.137920175,207.212229481,207.286558020,207.360905795,207.435272808,207.509659063,207.584064563,207.658489311,207.732933309,207.807396562,207.881879071,207.956380840,208.030901871,208.105442169,208.180001735,208.254580573,208.329178687,208.403796078,208.478432750,
		208.553088706,208.627763949,208.702458481,208.777172307,208.851905429,208.926657850,209.001429573,209.076220601,209.151030937,209.225860584,209.300709545,209.375577823,209.450465422,209.525372343,209.600298591,209.675244168,209.750209077,209.825193321,209.900196903,209.975219827,
		210.050262095,210.125323710,210.200404676,210.275504995,210.350624670,210.425763705,210.500922102,210.576099864,210.651296995,210.726513497,210.801749374,210.877004628,210.952279263,211.027573281,211.102886685,211.178219480,211.253571667,211.328943249,211.404334230,211.479744613,
		211.555174400,211.630623595,211.706092201,211.781580220,211.857087656,211.932614512,212.008160791,212.083726496,212.159311629,212.234916194,212.310540194,212.386183632,212.461846511,212.537528834,212.613230604,212.688951824,212.764692497,212.840452626,212.916232214,212.992031264,
		213.067849779,213.143687762,213.219545217,213.295422145,213.371318551,213.447234437,213.523169807,213.599124663,213.675099008,213.751092845,213.827106178,213.903139009,213.979191341,214.055263178,214.131354523,214.207465378,214.283595746,214.359745631,214.435915036,214.512103963,
		214.588312416,214.664540397,214.740787910,214.817054958,214.893341544,214.969647670,215.045973341,215.122318558,215.198683325,215.275067645,215.351471520,215.427894955,215.504337952,215.580800513,215.657282643,215.733784344,215.810305619,215.886846471,215.963406903,216.039986918,
		216.116586520,216.193205710,216.269844494,216.346502872,216.423180849,216.499878427,216.576595609,216.653332399,216.730088800,216.806864814,216.883660444,216.960475694,217.037310567,217.114165065,217.191039192,217.267932950,217.344846344,217.421779375,217.498732047,217.575704362,
		217.652696325,217.729707938,217.806739203,217.883790124,217.960860705,218.037950947,218.115060855,218.192190430,218.269339677,218.346508598,218.423697196,218.500905474,218.578133436,218.655381084,218.732648421,218.809935451,218.887242176,218.964568599,219.041914725,219.119280554,
		219.196666091,219.274071339,219.351496301,219.428940979,219.506405376,219.583889497,219.661393343,219.738916919,219.816460226,219.894023268,219.971606048,220.049208569,220.126830834,220.204472846,220.282134608,220.359816123,220.437517395,220.515238425,220.592979218,220.670739777,
		220.748520103,220.826320201,220.904140074,220.981979723,221.059839154,221.137718368,221.215617368,221.293536158,221.371474741,221.449433120,221.527411297,221.605409276,221.683427060,221.761464652,221.839522055,221.917599271,221.995696305,222.073813159,222.151949835,222.230106338,
		222.308282670,222.386478835,222.464694834,222.542930672,222.621186352,222.699461875,222.777757246,222.856072468,222.934407543,223.012762475,223.091137266,223.169531920,223.247946440,223.326380828,223.404835088,223.483309223,223.561803236,223.640317130,223.718850908,223.797404573,
		223.875978128,223.954571576,224.033184920,224.111818164,224.190471310,224.269144361,224.347837320,224.426550191,224.505282976,224.584035679,224.662808302,224.741600849,224.820413323,224.899245726,224.978098062,225.056970334,225.135862544,225.214774697,225.293706794,225.372658839,
		225.451630836,225.530622786,225.609634694,225.688666562,225.767718393,225.846790190,225.925881957,226.004993696,226.084125411,226.163277104,226.242448778,226.321640437,226.400852084,226.480083721,226.559335352,226.638606980,226.717898608,226.797210239,226.876541876,226.955893521,
		227.035265179,227.114656852,227.194068543,227.273500255,227.352951992,227.432423755,227.511915550,227.591427377,227.670959241,227.750511145,227.830083091,227.909675083,227.989287123,228.068919216,228.148571363,228.228243568,228.307935834,228.387648164,228.467380560,228.547133027,
		228.626905568,228.706698184,228.786510879,228.866343657,228.946196521,229.026069472,229.105962515,229.185875653,229.265808888,229.345762224,229.425735664,229.505729210,229.585742866,229.665776635,229.745830520,229.825904524,229.905998651,229.986112902,230.066247281,230.146401792,
		230.226576436,230.306771219,230.386986141,230.467221207,230.547476420,230.627751782,230.708047297,230.788362967,230.868698797,230.949054788,231.029430944,231.109827268,231.190243763,231.270680432,231.351137278,231.431614304,231.512111514,231.592628910,231.673166495,231.753724273,
		231.834302246,231.914900418,231.995518791,232.076157369,232.156816155,232.237495151,232.318194362,232.398913789,232.479653437,232.560413307,232.641193404,232.721993729,232.802814287,232.883655081,232.964516113,233.045397386,233.126298903,233.207220669,233.288162685,233.369124954,
		233.450107480,233.531110267,233.612133316,233.693176631,233.774240215,233.855324071,233.936428202,234.017552612,234.098697303,234.179862278,234.261047541,234.342253094,234.423478941,234.504725084,234.585991527,234.667278273,234.748585324,234.829912685,234.911260357,234.992628345,
		235.074016650,235.155425277,235.236854228,235.318303507,235.399773115,235.481263057,235.562773336,235.644303954,235.725854915,235.807426222,235.889017877,235.970629885,236.052262247,236.133914967,236.215588048,236.297281494,236.378995306,236.460729489,236.542484046,236.624258978,
		236.706054290,236.787869985,236.869706066,236.951562535,237.033439396,237.115336651,237.197254305,237.279192360,237.361150819,237.443129684,237.525128961,237.607148650,237.689188756,237.771249281,237.853330229,237.935431602,238.017553404,238.099695638,238.181858306,238.264041413,
		238.346244960,238.428468952,238.510713390,238.592978279,238.675263621,238.757569419,238.839895677,238.922242397,239.004609582,239.086997237,239.169405363,239.251833964,239.334283042,239.416752602,239.499242646,239.581753176,239.664284197,239.746835711,239.829407722,239.912000232,
		239.994613244,240.077246762,240.159900788,240.242575327,240.325270380,240.407985951,240.490722042,240.573478658,240.656255801,240.739053474,240.821871681,240.904710423,240.987569706,241.070449530,241.153349900,241.236270819,241.319212290,241.402174315,241.485156899,241.568160043,
		241.651183751,241.734228027,241.817292873,241.900378292,241.983484287,242.066610862,242.149758019,242.232925762,242.316114094,242.399323018,242.482552536,242.565802653,242.649073370,242.732364691,242.815676620,242.899009159,242.982362312,243.065736080,243.149130469,243.232545480,
		243.315981116
	};
}
