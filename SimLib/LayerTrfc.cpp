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
#include "LayerTrfc.h"
#include "ExceptionArgument.h"

namespace SimLib
{
	LayerTrfc::LayerTrfc(
		SimHandler&			sim,
		__uint16			port,
		TIDelegateSend&		delegateSend,
		TIDelegateRecv&		delegateRecv,
		TIDelegateAccept&	delegateAccept,
		TIDelegateAccepted&	delegateAccepted,
		uint				senderSegmentSize
		) : Layer(sim),
		delegateSend(delegateSend),
		delegateRecv(delegateRecv),
		delegateAccept(delegateAccept),
		delegateAccepted(delegateAccepted)
	{

		// Simulator
		this->sim = sim;

		// Parameters
		this->port = port;


		// Delegates (local)
		this->delegateDispose = alloc TDelegateDispose(*this, &LayerTrfc::Dispose);

		// Sender
		this->senderSegmentSize = senderSegmentSize;
	}

	LayerTrfc::~LayerTrfc()
	{
		// Delegates
		delete this->delegateDispose;
	}

	LayerTrfc::EResult LayerTrfc::Create(AddressIpv4 dstAddress, __uint16 dstPort, LayerTrfcReceiver** receiver)
	{
		// Create a receiver connection
		*receiver = alloc LayerTrfcReceiver(
			this->port,
			this->sim,
			this->delegateSend,
			this->delegateRecv,
			*this->delegateDispose,
			dstAddress,
			dstPort
			);

		// Add the connection to the connections list
		this->connections.insert(std::pair<uint, LayerTrfcConnection*>((*receiver)->Id(), *receiver));

		return SUCCESS;
	}

	LayerTrfc::EResult LayerTrfc::Recv(AddressIpv4 srcAddress, __uint16 srcPort, __uint16 dstPort, PacketTrfc* packet)
	{
		// Received connection packet

		// Check transport layer port
		if(dstPort != this->port) return FAIL_INCORRECT_PORT;

		// If the packet destination is not set
		if(packet->Dst() == PACKET_INVALID_CONNECTION)
		{
			// Check if the packet is a connection message
			if(packet->TypeConnection() == PacketTrfc::MESSAGE)
			{
				// Check if the packet is an OPEN message
				PacketTrfcMessage* message = type_cast<PacketTrfcMessage*>(packet);

				if(message->TypeMessage() == PacketTrfcMessage::OPEN)
				{
					// Send the OPEN request information to the client
					if(this->delegateAccept(srcAddress, packet->Payload()))
					{
						// Create a alloc sender connection
						LayerTrfcSender* connection = alloc LayerTrfcSender(
							this->port,
							this->sim,
							this->delegateSend,
							*this->delegateDispose,
							srcAddress,
							srcPort,
							message->Src(),
							this->senderSegmentSize
							);

						// Add the connection to the connections list
						this->connections.insert(std::pair<uint, LayerTrfcConnection*>(connection->Id(), connection));

						// Send the packet to the connection
						type_cast<LayerTrfcConnection*>(connection)->Recv(srcAddress, packet);

						// Send the connection to the upper layer
						this->delegateAccepted(connection);

						return SUCCESS;
					}
					else return FAIL_NOT_ACCEPTED;
				}
				else return FAIL_INVALID_DESTINATION_MESSAGE;
			}
			else return FAIL_INVALID_DESTINATION_PACKET;
		}
		else
		{
			// Get the connection
			TConnections::iterator iter = this->connections.find(packet->Dst());
			if(iter != this->connections.end())
			{
				// Pass the packet to the connection
				iter->second->Recv(srcAddress, packet);

				return SUCCESS;
			}
			else return FAIL_CONNECTION_NOT_EXIST;
		}
	}

	LayerTrfcConnection* LayerTrfc::Get(uint id)
	{
		// Check the connection exists
		TConnections::iterator iter = this->connections.find(id);
		if(iter == this->connections.end()) throw Exception(__FILE__, __LINE__, "LayerTrfc::Get - connection with ID %u does not exist.", id);

		return iter->second;
	}

	void LayerTrfc::Dispose(LayerTrfcConnection* connection)
	{
		// Check the connection exists
		TConnections::iterator iter = this->connections.find(connection->Id());
		if(iter == this->connections.end()) throw Exception(__FILE__, __LINE__, "LayerTrfc::Dispose - connection with ID %u does not exist.", connection->Id());

		// Remove the connection
		this->connections.erase(iter);

		// Delete the connection
		delete connection;
	}
}
