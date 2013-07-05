/* 
 * Copyright (C) 2012 Alex Bikfalvi
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

/*
0.0.0.0/8		Current network (only valid as source address)	RFC 1700
10.0.0.0/8		Private network	RFC 1918
100.64.0.0/10	Shared Address Space	RFC 6598
127.0.0.0/8		Loopback	RFC 5735
169.254.0.0/16	Link-local	RFC 3927
172.16.0.0/12	Private network	RFC 1918
192.0.0.0/24	IETF Protocol Assignments	RFC 5735
192.0.2.0/24	TEST-NET-1, documentation and examples	RFC 5735
192.88.99.0/24	IPv6 to IPv4 relay	RFC 3068
192.168.0.0/16	Private network	RFC 1918
198.18.0.0/15	Network benchmark tests	RFC 2544
198.51.100.0/24	TEST-NET-2, documentation and examples	RFC 5737
203.0.113.0/24	TEST-NET-3, documentation and examples	RFC 5737
224.0.0.0/4		IP multicast (former Class D network)	RFC 5771
240.0.0.0/4		Reserved (former Class E network)	RFC 1700
255.255.255.255	Broadcast	RFC 919
*/

#include "Address.h"

namespace SimLib
{
	class AddressIpv4 : public Address
	{
	public:
		enum
		{
			MCAST_ALL_SYSTEMS =	0xE0000001,
			MCAST_ALL_ROUTERS =	0xE0000002,
			MCAST_PIM_SM =		0xE000000D,
			ANY =				0xFFFFFFFE,
			INVALID =			0xFFFFFFFF
		};

	private:
		uint	address;
	
	public:
		AddressIpv4(uint address = INVALID) : Address(address), address(address) { }
		AddressIpv4(Address& address) : Address(address.Addr()), address(address.Addr()) { }
		AddressIpv4(const AddressIpv4& address) : Address(address), address(address.address) { }
		virtual ~AddressIpv4() { }

		static AddressIpv4	Multicast(uint group) { return 0xE0000000 | (group & 0x1FFFFFFF); }

		inline uint			Addr() { return this->address; }

		friend inline bool	operator<(const AddressIpv4& left, const AddressIpv4& right) { return left.address < right.address; }
		friend inline bool	operator==(const AddressIpv4& left, const AddressIpv4& right) { return left.address == right.address; }
		friend inline bool	operator!=(const AddressIpv4& left, const AddressIpv4& right) { return left.address != right.address; }
		
		inline void			operator=(const Address& address) { this->address = ((Address)address).Addr(); }

		inline bool			IsMulticast() { return (this->address & 0xF0000000) == 0xE0000000; }
		inline uint			MulticastGroup() { return this->address & 0x1FFFFFFF; }

		virtual inline bool	Equals(AddressIpv4 address) { return this->address == address.address; }
	};
}
