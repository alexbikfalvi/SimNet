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
#include "LayerTrfcConnection.h"
#include "Exception.h"

#pragma warning(disable : 4996)

namespace SimLib
{
	uint	LayerTrfcConnection::idGlobal = 0;
	uint	LayerTrfcConnection::flowGlobal = 0;

	__time	LayerTrfcConnection::timerOpenTimeout = 0.5;
	__time	LayerTrfcConnection::timerCloseTimeout = 0.5;
	__time	LayerTrfcConnection::timerCancelTimeout = 2.0;
	__time	LayerTrfcConnection::timerRequesterWaitTimeout = 2.0;
	__time	LayerTrfcConnection::timerResponderWaitTimeout = 120.0;
	__time	LayerTrfcConnection::timerDefaultWaitTimeout = 30.0;

	char*	LayerTrfcConnection::strConnectionType[] = {"REQUESTER", "RESPONDER"};
	char*	LayerTrfcConnection::strState[] = {"CLOSED", "OPENING", "OPENED", "CLOSING", "CANCELING", "WAITING"};

	LayerTrfcConnection::LayerTrfcConnection(
		EConnectionType		type,
		__uint16			port,
		SimHandler&			sim,
		TIDelegateSend&		delegateSend,
		TIDelegateDispose&	delegateDispose,
		AddressIpv4			remoteAddress,
		__uint16			remotePort
		) : Layer(sim), delegateSend(delegateSend), delegateDispose(delegateDispose)
	{
		// Type
		this->type = type;

		// Flow
		this->flow = 0xFFFFFFFF;

		// State
		this->state = CLOSED;

		// Local
		this->id = LayerTrfcConnection::idGlobal++;
		this->port = port;

		// Remote
		this->remoteAddress = remoteAddress;
		this->remoteId = 0;
		this->remotePort = remotePort;

		// Events
		this->eventOpen = alloc Event2<void, LayerTrfcConnection*, EOpenResult>();
		this->eventClose = alloc Event2<void, LayerTrfcConnection*, ECloseResult>();

		// Timers
		this->timerControl = alloc SimTimer<LayerTrfcConnection>(this->sim, *this);

		// Tag
		this->tag = NULL;

		// Set functions
		switch(this->type)
		{
		case REQUESTER:
			this->functionOpen = &LayerTrfcConnection::OpenRequester;
			this->functionClose = &LayerTrfcConnection::CloseRequester;
			this->functionRecvMessageOpen = &LayerTrfcConnection::RecvMessageOpenRequester;
			this->functionRecvMessageOpenAck = &LayerTrfcConnection::RecvMessageOpenAckRequester;
			this->functionRecvMessageClose = &LayerTrfcConnection::RecvMessageCloseRequester;
			this->functionRecvMessageCloseAck = &LayerTrfcConnection::RecvMessageCloseAckRequester;
			this->functionRecvMessageCloseAckAck = &LayerTrfcConnection::RecvMessageCloseAckAckRequester;
			break;
		case RESPONDER:
			this->functionOpen = &LayerTrfcConnection::OpenResponder;
			this->functionClose = &LayerTrfcConnection::CloseResponder;
			this->functionRecvMessageOpen = &LayerTrfcConnection::RecvMessageOpenResponder;
			this->functionRecvMessageOpenAck = &LayerTrfcConnection::RecvMessageOpenAckResponder;
			this->functionRecvMessageClose = &LayerTrfcConnection::RecvMessageCloseResponder;
			this->functionRecvMessageCloseAck = &LayerTrfcConnection::RecvMessageCloseAckResponder;
			this->functionRecvMessageCloseAckAck = &LayerTrfcConnection::RecvMessageCloseAckAckResponder;
			break;
		}
	}

	LayerTrfcConnection::~LayerTrfcConnection()
	{
		// Events
		delete this->eventOpen;
		delete this->eventClose;

		// Timers
		delete this->timerControl;
	}

	void LayerTrfcConnection::Open()
	{
		// Call open function for connection type
		(this->*this->functionOpen)();
	}

	void LayerTrfcConnection::OpenRequester()
	{
		// Exception for any connection state other that CLOSED
		if(CLOSED != this->state) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::OpenRequester - curernt state is not CLSOED (%u).", this->state);

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_RED);
		printf("\n\tT = %8.3lf\tOpenRequester() : %s -> ", this->sim.Time(), this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif

		// Initialize the flow number
		this->flow = LayerTrfcConnection::flowGlobal++;

		// Change the state
		this->state = OPENING;

		// Send an OPEN message to the sender
		PacketTrfcMessage* packet = alloc PacketTrfcMessage(
			this->flow,
			this->id,
			this->sim.Time());

		this->delegateSend(this->port, this->remotePort, this->remoteAddress, 128, packet);

		// Set the control timer to open handler
		this->timerControl->SetAfter(LayerTrfcConnection::timerOpenTimeout, &LayerTrfcConnection::TimerOpen);

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_RED);
		printf("%s", this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif
	}

	void LayerTrfcConnection::OpenResponder()
	{
#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_BLUE);
		printf("\n\tT = %8.3lf\tOpenResponder() : %s -> ", this->sim.Time(), this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif

		// Exception for any connection state
		throw Exception(__FILE__, __LINE__, "To do.");

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_BLUE);
		printf("%s", this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif
	}

	void LayerTrfcConnection::Close()
	{
		// Call close function for connection type
		(this->*this->functionClose)();
	}

	void LayerTrfcConnection::CloseRequester()
	{
#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_RED);
		printf("\n\tT = %8.3lf\tCloseRequester() : %s -> ", this->sim.Time(), this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif

		// Close requester
		switch(this->state)
		{
		case OPENING:						// Change to CANCELING / none / timer cancel
			{
				// Check the control timer is set
				if(!this->timerControl->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::Close - control timer must be set.");

				// Cancel the control timer
				this->timerControl->Cancel();

				// Change the state
				this->state = CANCELING;

				// Raise open event
				(*this->eventOpen)(this, OPEN_CANCELED);

				// Set the control timer to cancel handler
				this->timerControl->SetAfter(LayerTrfcConnection::timerCancelTimeout, &LayerTrfcConnection::TimerCancel);
			}
			break;
		case OPENED:						// Change to CLOSING / CLOSE / timer close
			// If connection is opened, send a CLOSE message to remote party
			{
				// Check the control timer is not set
				if(this->timerControl->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::Close - control timer must not be set.");

				// Send a CLOSE message to the sender
				PacketTrfcMessage* packet = alloc PacketTrfcMessage(
					this->flow,
					this->id,
					this->remoteId,
					PacketTrfcMessage::CLOSE,
					this->sim.Time()
					);

				this->delegateSend(this->port, this->remotePort, this->remoteAddress, 128, packet);

				// Change the state
				this->state = CLOSING;

				// Set the control timer to close handler
				this->timerControl->SetAfter(LayerTrfcConnection::timerCloseTimeout, &LayerTrfcConnection::TimerClose);
			}
			break;
		case CLOSING: break;				// Ignore
		case WAITING: break;				// Ignore
		default: throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::Close - invalid state (%u)", this->state); // Exception for all other states
		}

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_RED);
		printf("%s", this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif
	}

	void LayerTrfcConnection::CloseResponder()
	{
#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_BLUE);
		printf("\n\tT = %8.3lf\tCloseResponder() : %s -> ", this->sim.Time(), this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif

		// Close responder
		switch(this->state)
		{
		case OPENING:						// Change to CLOSING / none / timer cancel
			// If connection is opened, send a CLOSE message to remote party
			{
				// Check the control timer is set
				if(!this->timerControl->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::CloseResponder - control timer must be set.");

				// Cancel the control timer
				this->timerControl->Cancel();

				// Send a CLOSE message to the sender
				PacketTrfcMessage* packet = alloc PacketTrfcMessage(
					this->flow,
					this->id,
					this->remoteId,
					PacketTrfcMessage::CLOSE,
					this->sim.Time()
					);

				this->delegateSend(this->port, this->remotePort, this->remoteAddress, 128, packet);

				// Set the control timer to close handler
				this->timerControl->SetAfter(LayerTrfcConnection::timerCloseTimeout, &LayerTrfcConnection::TimerClose);

				// Change the state
				this->state = CLOSING;

				// Raise open event
				(*this->eventOpen)(this, OPEN_CANCELED);
			}
			break;
		case OPENED:						// Change to CLOSING / CLOSE / timer close
			// If connection is opened, send a CLOSE message to remote party
			{
				// Check the control timer is not set
				if(this->timerControl->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::CloseResponder - control timer must not be set.");

				// Send a CLOSE message to the sender
				PacketTrfcMessage* packet = alloc PacketTrfcMessage(
					this->flow,
					this->id,
					this->remoteId,
					PacketTrfcMessage::CLOSE,
					this->sim.Time()
					);

				this->delegateSend(this->port, this->remotePort, this->remoteAddress, 128, packet);

				// Change the state
				this->state = CLOSING;

				// Set the control timer to close handler
				this->timerControl->SetAfter(LayerTrfcConnection::timerCloseTimeout, &LayerTrfcConnection::TimerClose);
			}
			break;
		case CLOSING: break;				// Ignore in CLOSING state
		case WAITING: break;				// Ignore in WAITING state
		default: throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::CloseResponder - invalid state (%u).", this->state);	// Exception for other states
		}

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_BLUE);
		printf("%s", this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif
	}

	bool LayerTrfcConnection::SendData(PacketTrfcData* packet)
	{
		throw Exception(__FILE__, __LINE__, "To do");

		return false;
	}

	void LayerTrfcConnection::Recv(AddressIpv4 src, PacketTrfc* packet)
	{
		// Check the packet source and destination
		if(src != this->remoteAddress) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::Recv - source (%u) different from the remote address (%u).", src.Addr(), this->remoteAddress.Addr());

		switch(packet->TypeConnection())
		{
		case PacketTrfc::MESSAGE: this->RecvMessage(type_cast<PacketTrfcMessage*>(packet)); break;
		case PacketTrfc::FEEDBACK: this->RecvFeedback(type_cast<PacketTrfcFeedback*>(packet)); break;
		case PacketTrfc::DATA: this->RecvData(type_cast<PacketTrfcData*>(packet)); break;
		default: throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::Recv - invalid connection type (%u).", packet->TypeConnection());
		}

		// General processing
		this->Recv(packet);
	}

	void LayerTrfcConnection::RecvMessage(PacketTrfcMessage* packet)
	{
		switch(packet->TypeMessage())
		{
		case PacketTrfcMessage::OPEN: (this->*this->functionRecvMessageOpen)(packet); break;
		case PacketTrfcMessage::OPEN_ACK: (this->*this->functionRecvMessageOpenAck)(packet); break;
		case PacketTrfcMessage::CLOSE: (this->*this->functionRecvMessageClose)(packet); break;
		case PacketTrfcMessage::CLOSE_ACK: (this->*this->functionRecvMessageCloseAck)(packet); break;
		case PacketTrfcMessage::CLOSE_ACK_ACK: (this->*this->functionRecvMessageCloseAckAck)(packet); break;
		default: throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessage - invalid message type (%u).", packet->TypeMessage());
		}
	}

	void LayerTrfcConnection::RecvMessageOpenRequester(PacketTrfcMessage* packet)
	{
		// Received an OPEN message

		// Exception for any connection state
		throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageOpenRequester - cannot receive OPEN message.");
	}

	void LayerTrfcConnection::RecvMessageOpenAckRequester(PacketTrfcMessage* packet)
	{
		// Received an OPEN-ACK message

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_RED);
		printf("\n\tT = %8.3lf\tRecvRequester(OPEN_ACK) : %s -> ", this->sim.Time(), this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif

		// Check the packet source and destination
		if(packet->Dst() == this->id) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageOpenAckRequester - destination mismatch.");
		if(packet->Flow() == this->flow) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageOpenAckRequester - flow mismatch.");

		// Check the state
		switch(this->state)
		{
		case LayerTrfcConnection::OPENING:		// Change to OPENED / OPEN_ACK / none
			{
				// Check the control timer is set
				if(!this->timerControl->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageOpenAckRequester - control timer must be set.");

				// Cancel the control timer
				this->timerControl->Cancel();

				// Set the sender information
				this->remoteId = packet->Src();

				// Send a OPEN-ACK message
				PacketTrfcMessage* reply = alloc PacketTrfcMessage(
					this->flow,
					this->id,
					this->remoteId,
					PacketTrfcMessage::OPEN_ACK,
					this->sim.Time(),
					packet->TimeTx()
					);

				this->delegateSend(this->port, this->remotePort, this->remoteAddress, 128, reply);

				// Change the state
				this->state = OPENED;

				// Raise open event
				(*this->eventOpen)(this, OPEN_SUCCESS);
			}
			break;
		case LayerTrfcConnection::CANCELING:	// Change to CLOSING / CLOSE / timer close
			{
				// Check the control timer is set
				if(!this->timerControl->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageOpenAckRequester - control timer must be set.");

				// Cancel the control timer
				this->timerControl->Cancel();

				// Set the sender information
				this->remoteId = packet->Src();

				// Send a CLOSE message
				PacketTrfcMessage* reply = alloc PacketTrfcMessage(
					this->flow,
					this->id,
					this->remoteId,
					PacketTrfcMessage::CLOSE,
					this->sim.Time(),
					packet->TimeTx()
					);

				this->delegateSend(this->port, this->remotePort, this->remoteAddress, 128, reply);

				// Change the state
				this->state = CLOSING;

				// Set the control timer to close handler
				this->timerControl->SetAfter(LayerTrfcConnection::timerCloseTimeout, &LayerTrfcConnection::TimerClose);
			}
			break;
		default:						// Exception for other states
			throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageOpenAckRequester - invalid state (%u).", this->state);
		}

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_RED);
		printf("%s", this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif
	}

	void LayerTrfcConnection::RecvMessageCloseRequester(PacketTrfcMessage* packet)
	{
		// Received a CLOSE message

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_RED);
		printf("\n\tT = %8.3lf\tRecvRequester(CLOSE) : %s -> ", this->sim.Time(), this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif

		// Check the packet source and destination
		if(packet->Dst() != this->id) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseRequester - destination mismatch.");
		if(packet->Flow() != this->flow) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseRequester - flow mismatch.");

		// Check the state
		switch(this->state)
		{
		case OPENING:					// Change to CLOSING / CLOSE_ACK / timer close
			{
				// Check the control timer is set
				if(!this->timerControl->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseRequester - control timer must be set.");

				// Cancel the control timer
				this->timerControl->Cancel();

				// Send a CLOSE-ACK message
				PacketTrfcMessage* reply = alloc PacketTrfcMessage(
					this->flow,
					this->id,
					this->remoteId,
					PacketTrfcMessage::CLOSE_ACK,
					this->sim.Time(),
					packet->TimeTx()
					);

				this->delegateSend(this->port, this->remotePort, this->remoteAddress, 128, reply);

				// Change the state
				this->state = CLOSING;

				// Raise the event
				(*this->eventOpen)(this, OPEN_FAIL_REMOTE);

				// Set the control timer to close handler
				this->timerControl->SetAfter(LayerTrfcConnection::timerCloseTimeout, &LayerTrfcConnection::TimerClose);
			}
			break;
		case OPENED:					// Change to CLOSING / CLOSE_ACK / timer close
			{
				// Check the control timer is not set
				if(this->timerControl->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseRequester - control timer must not be set.");

				// Send a CLOSE-ACK message
				PacketTrfcMessage* reply = alloc PacketTrfcMessage(
					this->flow,
					this->id,
					this->remoteId,
					PacketTrfcMessage::CLOSE_ACK,
					this->sim.Time(),
					packet->TimeTx()
					);

				this->delegateSend(this->port, this->remotePort, this->remoteAddress, 128, reply);

				// Change the state
				this->state = CLOSING;

				// Set the control timer to close handler
				this->timerControl->SetAfter(LayerTrfcConnection::timerCloseTimeout, &LayerTrfcConnection::TimerClose);
			}
			break;
		case CANCELING:					// Change to CLOSING / CLOSE_ACK / timer close
			{
				// Check the control timer is set
				if(!this->timerControl->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseRequester - control timer must be set.");

				// Cancel the control timer
				this->timerControl->Cancel();

				// Send a CLOSE-ACK message
				PacketTrfcMessage* reply = alloc PacketTrfcMessage(
					this->flow,
					this->id,
					this->remoteId,
					PacketTrfcMessage::CLOSE_ACK,
					this->sim.Time(),
					packet->TimeTx()
					);

				this->delegateSend(this->port, this->remotePort, this->remoteAddress, 128, reply);

				// Change the state
				this->state = CLOSING;

				// Set the control timer to close handler
				this->timerControl->SetAfter(LayerTrfcConnection::timerCloseTimeout, &LayerTrfcConnection::TimerClose);
			}
			break;
		case CLOSING:					// Change to CLOSING / CLOSE_ACK / none
			{
				// Check the control timer is set
				if(!this->timerControl->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseRequester - control timer must be set.");

				// Send a CLOSE-ACK message
				PacketTrfcMessage* reply = alloc PacketTrfcMessage(
					this->flow,
					this->id,
					this->remoteId,
					PacketTrfcMessage::CLOSE_ACK,
					this->sim.Time(),
					packet->TimeTx()
					);

				this->delegateSend(this->port, this->remotePort, this->remoteAddress, 128, reply);

				// Leave the current state and timer
			}
			break;
		default:						// Exception for other states
			throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseRequester - invalid state (%u).", this->state);
		}

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_RED);
		printf("%s", this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif
	}

	void LayerTrfcConnection::RecvMessageCloseAckRequester(PacketTrfcMessage* packet)
	{
		// Received a CLOSE-ACK message

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_RED);
		printf("\n\tT = %8.3lf\tRecvRequester(CLOSE_ACK) : %s -> ", this->sim.Time(), this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif

		// Check the packet source and destination
		if(packet->Dst() != this->id) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseAckRequester - destination mismatch.");
		if(packet->Flow() != this->flow) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseAckRequester - flow mismatch.");

		// Check the state
		switch(this->state)
		{
		case CLOSING:					// Change to WAITING / CLOSE_ACK_ACK / timer wait
			{
				// Check the control timer is set
				if(!this->timerControl->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseAckRequester - control timer must be set.");

				// Cancel the control timer
				this->timerControl->Cancel();

				// Send a CLOSE-ACK-ACK message
				PacketTrfcMessage* reply = alloc PacketTrfcMessage(
					this->flow,
					this->id,
					this->remoteId,
					PacketTrfcMessage::CLOSE_ACK_ACK,
					this->sim.Time(),
					packet->TimeTx()
					);

				this->delegateSend(this->port, this->remotePort, this->remoteAddress, 128, reply);

				// Change the state
				this->state = WAITING;

				// Raise the event
				(*this->eventClose)(this, CLOSE_CONFIRMED);

				// Set the control timer to the waiting handler
				this->timerControl->SetAfter(LayerTrfcConnection::timerRequesterWaitTimeout, &LayerTrfcConnection::TimerWait);
			}
			break;
		default:						// Exception for other states
			throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseAckRequester - invalid state (%u).", this->state);
		}

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_RED);
		printf("%s", this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif
	}

	void LayerTrfcConnection::RecvMessageCloseAckAckRequester(PacketTrfcMessage* packet)
	{
		// Received a CLOSE-ACK-ACK

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_RED);
		printf("\n\tT = %8.3lf\tRecvRequester(CLOSE_ACK_ACK) : %s -> ", this->sim.Time(), this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif

		// Check the packet source and destination
		if(packet->Dst() != this->id) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseAckAckRequester - destination mismatch.");
		if(packet->Flow() != this->flow) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseAckAckRequester - flow mismatch.");

		// Check the state
		switch(this->state)
		{
		case CLOSING:					// Change to WAITING / none / timer wait
			{
				// Check the control timer is set
				if(!this->timerControl->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseAckAckRequester - control timer must be set.");

				// Cancel the control timer
				this->timerControl->Cancel();

				// Change the state
				this->state = WAITING;

				// Raise the event
				(*this->eventClose)(this, CLOSE_CONFIRMED);

				// Set the control timer to the waiting handler
				this->timerControl->SetAfter(LayerTrfcConnection::timerResponderWaitTimeout, &LayerTrfcConnection::TimerWait);
			}
			break;
		case CLOSED:
		case WAITING:
			break;						// Ignore in CLOSED or WAITING state
		default:						// Exception for other states
			throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseAckAckRequester - invalid state (%u).", this->state);
		}

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_RED);
		printf("%s", this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif
	}

	void LayerTrfcConnection::RecvMessageOpenResponder(PacketTrfcMessage* packet)
	{
		// Received an OPEN message

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_BLUE);
		printf("\n\tT = %8.3lf\tRecvResponder(OPEN) : %s -> ", this->sim.Time(), this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif

		// Check the packet source and destination : must be set to an invalid connection
		if(packet->Dst() != PACKET_INVALID_CONNECTION) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageOpenResponder - destination mismatch.");

		// Check the state
		switch(this->state)
		{
		case CLOSED:					// Change to OPENING / OPEN_ACK / timer open
			{
				// Check the control timer is not set
				if(this->timerControl->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageOpenResponder - control timer must not be set.");

				// Set the flow number
				this->flow = packet->Flow();

				// Send an OPEN-ACK message
				PacketTrfcMessage* reply = alloc PacketTrfcMessage(
					this->flow,
					this->id,
					this->remoteId,
					PacketTrfcMessage::OPEN_ACK,
					this->sim.Time(),
					packet->TimeTx()
					);

				this->delegateSend(this->port, this->remotePort, this->remoteAddress, 128, reply);

				// Change the state
				this->state = OPENING;

				// Set the control timer to open handler
				this->timerControl->SetAfter(LayerTrfcConnection::timerOpenTimeout, &LayerTrfcConnection::TimerOpen);
			}
			break;
		default:						// Exception for other state
			throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageOpenResponder - invalid state (%u).", this->state);
		}

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_BLUE);
		printf("%s", this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif
	}

	void LayerTrfcConnection::RecvMessageOpenAckResponder(PacketTrfcMessage* packet)
	{
		// Received an OPEN-ACK message

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_BLUE);
		printf("\n\tT = %8.3lf\tRecvResponder(OPEN_ACK) : %s -> ", this->sim.Time(), this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif

		// Check the packet source and destination
		if(packet->Dst() != this->id) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageOpenAckResponder - destination mismatch.");
		if(packet->Flow() != this->flow) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageOpenAckResponder - flow mismatch.");

		// Check the state
		switch(this->state)
		{
		case OPENING:					// Change to OPENED / none / none
			{
				// Check the control timer is set
				if(!this->timerControl->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseResponder - control timer must be set.");

				// Cancel the control timer
				this->timerControl->Cancel();

				// Change the state
				this->state = OPENED;

				// Raise the event
				(*this->eventOpen)(this, OPEN_SUCCESS);
			}
			break;
		case CLOSING: break;			// Ignore in CLOSING state
		default:						// Exception for other state
			throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageOpenAckResponder - invalid state (%u).", this->state);
		}

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_BLUE);
		printf("%s", this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif
	}

	void LayerTrfcConnection::RecvMessageCloseResponder(PacketTrfcMessage* packet)
	{
		// Received a CLOSE message

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_BLUE);
		printf("\n\tT = %8.3lf\tRecvResponder(CLOSE) : %s -> ", this->sim.Time(), this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif

		// Check the packet source and destination
		if(packet->Dst() != this->id) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseResponder - destination mismatch.");
		if(packet->Flow() != this->flow) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseResponder - flow mismatch.");

		// Check the state
		switch(this->state)
		{
		case OPENING:					// Change to CLOSING / CLOSE_ACK / timer close
			{
				// Check the control timer is set
				if(!this->timerControl->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseResponder - control timer must be set.");

				// Cancel the control timer
				this->timerControl->Cancel();

				// Send a CLOSE-ACK message
				PacketTrfcMessage* reply = alloc PacketTrfcMessage(
					this->flow,
					this->id,
					this->remoteId,
					PacketTrfcMessage::CLOSE_ACK,
					this->sim.Time(),
					packet->TimeTx()
					);

				this->delegateSend(this->port, this->remotePort, this->remoteAddress, 128, reply);

				// Change the state
				this->state = CLOSING;

				// Raise the event
				(*this->eventOpen)(this, OPEN_FAIL_REMOTE);

				// Set the control timer to close handler
				this->timerControl->SetAfter(LayerTrfcConnection::timerCloseTimeout, &LayerTrfcConnection::TimerClose);
			}
			break;
		case OPENED:					// Change to CLOSING / CLOSE_ACK / timer close
			{
				// Check the control timer is not set
				if(this->timerControl->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseResponder - control timer must not be set.");

				// Send a CLOSE-ACK message
				PacketTrfcMessage* reply = alloc PacketTrfcMessage(
					this->flow,
					this->id,
					this->remoteId,
					PacketTrfcMessage::CLOSE_ACK,
					this->sim.Time(),
					packet->TimeTx()
					);
				this->delegateSend(this->port, this->remotePort, this->remoteAddress, 128, reply);

				// Change the state
				this->state = CLOSING;

				// Set the control timer to close handler
				this->timerControl->SetAfter(LayerTrfcConnection::timerCloseTimeout, &LayerTrfcConnection::TimerClose);
			}
			break;
		case CLOSING:					// Change to CLOSING / CLOSE_ACK / none
			{
				// Check the control timer is set
				if(!this->timerControl->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseResponder - control timer must be set.");

				// Send a CLOSE-ACK message
				PacketTrfcMessage* reply = alloc PacketTrfcMessage(
					this->flow,
					this->id,
					this->remoteId,
					PacketTrfcMessage::CLOSE_ACK,
					this->sim.Time(),
					packet->TimeTx()
					);
				this->delegateSend(this->port, this->remotePort, this->remoteAddress, 128, reply);

				// Leave the current state and timer
			}
			break;
		default:						// Exception for other states
			throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseResponder - invalid state (%u).", this->state);
		}

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_BLUE);
		printf("%s", this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif
	}

	void LayerTrfcConnection::RecvMessageCloseAckResponder(PacketTrfcMessage* packet)
	{
		// Received a CLOSE-ACK message

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_BLUE);
		printf("\n\tT = %8.3lf\tRecvResponder(CLOSE_ACK) : %s -> ", this->sim.Time(), this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif

		// Check the packet source and destination
		if(packet->Dst() != this->id) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseAckResponder - destination mismatch.");
		if(packet->Flow() != this->flow) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseAckResponder - flow mismatch.");

		// Check the state
		switch(this->state)
		{
		case CLOSING:					// Change to WAITING / CLOSE_ACK_ACK / timer wait
			{
				// Check the control timer is set
				if(!this->timerControl->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseAckResponder - control timer must be set.");

				// Cancel the control timer
				this->timerControl->Cancel();

				// Send a CLOSE-ACK-ACK message
				PacketTrfcMessage* reply = alloc PacketTrfcMessage(
					this->flow,
					this->id,
					this->remoteId,
					PacketTrfcMessage::CLOSE_ACK_ACK,
					this->sim.Time(),
					packet->TimeTx()
					);
				this->delegateSend(this->port, this->remotePort, this->remoteAddress, 128, reply);

				// Change the state
				this->state = WAITING;

				// Raise the event
				(*this->eventClose)(this, CLOSE_CONFIRMED);

				// Set the control timer to the waiting handler
				this->timerControl->SetAfter(LayerTrfcConnection::timerResponderWaitTimeout, &LayerTrfcConnection::TimerWait);
			}
			break;
		default:					// Exception for other sattes
			throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseAckResponder - invalid state (%u).", this->state);
		}

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_BLUE);
		printf("%s", this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif
	}

	void LayerTrfcConnection::RecvMessageCloseAckAckResponder(PacketTrfcMessage* packet)
	{
		// Received a CLOSE-ACK-ACK message

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_BLUE);
		printf("\n\tT = %8.3lf\tRecvResponder(CLOSE_ACK_ACK) : %s -> ", this->sim.Time(), this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif

		// Check the packet source and destination
		if(packet->Dst() != this->id) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseAckAckResponder - destination mismatch.");
		if(packet->Flow() != this->flow) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseAckAckResponder - flow mismatch.");

		// Check the state
		switch(this->state)
		{
		case CLOSING:					// Change to WAITING / none / timer wait
			{
				// Check the control timer is set
				if(!this->timerControl->IsSet()) throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseAckAckResponder - control timer must be set.");

				// Cancel the control timer
				this->timerControl->Cancel();

				// Change the state
				this->state = WAITING;

				// Raise the event
				(*this->eventClose)(this, CLOSE_CONFIRMED);

				// Set the control timer to the waiting handler
				this->timerControl->SetAfter(LayerTrfcConnection::timerRequesterWaitTimeout, &LayerTrfcConnection::TimerWait);
			}
			break;
		case CLOSED:
		case WAITING:
			break;						// Ignore in CLOSED and WAITING state
		default:						// Exception for other states
			throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::RecvMessageCloseAckAckResponder - invalid state (%u).", this->state);
		}

#ifdef LOG_FLOW
		Console::SetColor(Console::DARK_BLUE);
		printf("%s", this->ToString());
		Console::SetColor(Console::LIGHT_GRAY);
#endif
	}

	void LayerTrfcConnection::TimerOpen(SimTimerInfo* info)
	{
		// Check the state
		switch(this->state)
		{
		case OPENING:					// Change to CLOSED / none / none
			{
				// Change the state
				this->state = CLOSED;

				// Raise the event
				(*this->eventOpen)(this, OPEN_FAIL_TIMEOUT);

				// Call the dispose function to relase the resources for this connection
				this->delegateDispose(this);
			}
			break;
		default:						// Exception for other states
			throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::TimerOpen - invalid state (%u)", this->state);
		}
	}

	void LayerTrfcConnection::TimerClose(SimTimerInfo* info)
	{
		// Check the state
		switch(this->state)
		{
		case CLOSING:					// Change to WAITING / none / timer wait
			{
				// Change the state
				this->state = WAITING;

				// Raise the event
				(*this->eventClose)(this, CLOSE_TIMEOUT);

				// Set the control timer to the waiting handler
				this->timerControl->SetAfter(LayerTrfcConnection::timerDefaultWaitTimeout, &LayerTrfcConnection::TimerWait);
			}
			break;
		default:						// Exception for other states
			throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::TimerClose - invalid state (%u).", this->state);
		}
	}

	void LayerTrfcConnection::TimerCancel(SimTimerInfo* info)
	{
		// Check the state
		switch(this->state)
		{
		case CANCELING:					// Change to CLOSED / none / none
			{
				// Change the state
				this->state = CLOSED;

				// Raise the event
				(*this->eventClose)(this, CLOSE_TIMEOUT);

				// Call the dispose function to relase the resources for this connection
				this->delegateDispose(this);
			}
			break;
		default:						// Exception for other states
			throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::TimerCancel - invalid state (%u).", this->state);
		}
	}

	void LayerTrfcConnection::TimerWait(SimTimerInfo* info)
	{
		// Check the state
		switch(this->state)
		{
		case WAITING:					// Change to CLOSED / none / none
			{
				// Change the state
				this->state = CLOSED;

				// Raise the event
				(*this->eventClose)(this, CLOSE_COMPLETE);

				// Call the dispose function to release the resources for this connection
				this->delegateDispose(this);
			}
			break;
		default:						// Exception for other states
			throw Exception(__FILE__, __LINE__, "LayerTrfcConnection::TimerWait - invalid state (%u).", this->state);
		}
	}

	char* LayerTrfcConnection::ToString() const
	{
		sprintf((char*)this->str, "[ l=*:%u:%u r=%u:%u:%u t=%s s=%s ]",
			this->port, this->id, this->remoteAddress, this->remotePort, this->remoteId,
			LayerTrfcConnection::strConnectionType[this->type], LayerTrfcConnection::strState[this->state]
			);

		return (char*)this->str;
	}
}
