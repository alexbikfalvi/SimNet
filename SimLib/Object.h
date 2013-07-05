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
#include "Packet.h"

namespace SimLib
{
	class Object
	{
	public:
		enum ERecvCode
		{
			RECV_SUCCESS = 0,
			RECV_FAIL = 1
		};

	protected:
		SimHandler&	sim;

	public:
		Object(
			SimHandler&	sim
			);
		virtual ~Object() { }

		virtual	ERecvCode	Recv(Object* sender, uint entry, Packet* packet) = 0;
		
		virtual void		Initialize() = 0;
		virtual void		Finalize() = 0;
	};
}