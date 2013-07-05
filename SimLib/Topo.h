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

#include "TopoNode.h"
#include "TopoEdge.h"
#include "TopoRoute.h"

namespace SimLib
{
	class Topo
	{
	public:
		Topo() { }
		virtual ~Topo() { }

		virtual uint		Nodes() = 0;
		virtual TopoNode*	Node(uint index) = 0;

		virtual uint		Edges() = 0;
		virtual TopoEdge*	Edge(uint index) = 0;

		virtual TopoRoute*	Route() = 0;
	};
}
