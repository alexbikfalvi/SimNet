#include "Headers.h"
#include "PacketPimSm.h"

namespace SimLib
{
	PacketPimSm::PacketPimSm(
		EPacketPimSmType	type,
		uint			size,
		Packet*				payload,
		ETypeOfService		tos
		) : Packet(size + PACKET_PIM_SM_HEADER, payload, tos)
	{
		this->type = type;
	}
}