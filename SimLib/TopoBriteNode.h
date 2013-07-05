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

namespace SimLib
{
	class TopoBriteNode : public TopoNode
	{
	private:
		uint	id;
		uint	xpos;
		uint	ypos;
		uint	degree;

		uint*	edges;
		uint	edgeSet;

	public:
		TopoBriteNode(
			uint	id,
			uint	xpos,
			uint	ypos,
			uint	degree
			);
		~TopoBriteNode();

		inline uint	Id() { return this->id; }
		inline uint	XPos() { return this->xpos; }
		inline uint	YPos() { return this->ypos; }
		inline uint	Degree() { return this->degree; }
		inline uint	Edge(uint index) { return this->edges[index]; }
		uint		SetEdge(uint edge);
	};
}
