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

#include "SimHandler.h"
#include "Layer.h"
#include "LayerTrfcSender.h"
#include "LayerTrfcReceiver.h"
#include "AddressIpv4.h"
#include "PacketTrfc.h"

namespace SimLib
{
	class LayerTrfc : public Layer
	{
	public:
		enum EResult
		{
			SUCCESS = 0,							// Packet received and processed successfully
			FAIL_INCORRECT_PORT = 1,				// Packet arrived on the incorrect port
			FAIL_INVALID_DESTINATION_PACKET = 2,	// Packet with invalid destination is not a message packet
			FAIL_INVALID_DESTINATION_MESSAGE = 3,	// Message packet with invalid destination is not an OPEN message
			FAIL_CONNECTION_NOT_EXIST = 4,			// Packet destination is valid, but the connection does not exist,
			FAIL_NOT_ACCEPTED = 5					// The upper layer did not accept the connection request
		};

		typedef std::map<uint, LayerTrfcConnection*>								TConnections;
		typedef IDelegate5<void, __uint16, __uint16, AddressIpv4, __byte, Packet*>	TIDelegateSend;
		typedef IDelegate2<void, LayerTrfcReceiver*, Packet*>						TIDelegateRecv;
		typedef IDelegate2<bool, AddressIpv4, Packet*>								TIDelegateAccept;
		typedef IDelegate1<void, LayerTrfcSender*>									TIDelegateAccepted;
		typedef Delegate1<LayerTrfc, void, LayerTrfcConnection*>					TDelegateDispose;

	private:
		// Connections
		TConnections		connections;

		// Port
		__uint16			port;
		
		// Delegates
		TIDelegateSend&		delegateSend;
		TIDelegateRecv&		delegateRecv;
		TIDelegateAccept&	delegateAccept;
		TIDelegateAccepted&	delegateAccepted;

		TDelegateDispose*	delegateDispose;

		// Sender
		uint				senderSegmentSize;

	public:
		LayerTrfc(
			SimHandler&			sim,
			__uint16			port,
			TIDelegateSend&		delegateSend,
			TIDelegateRecv&		delegateRecv,
			TIDelegateAccept&	delegateAccept,
			TIDelegateAccepted&	delegateAccepted,
			uint				senderSegmentSize
			);
		virtual ~LayerTrfc();

		EResult					Recv(AddressIpv4 srcAddress, __uint16 srcPort, __uint16 dstPort, PacketTrfc* packet);
		EResult					Create(AddressIpv4 dstAddress, __uint16 dstPort, LayerTrfcReceiver** receiver);

		LayerTrfcConnection*	Get(uint id);

		virtual void			Initialize() { }
		virtual void			Finalize() { }

	private:
		void					Dispose(LayerTrfcConnection* connection);
	};
 }