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
#include "TopoBriteNode.h"

namespace SimLib
{
	TopoBriteNode::TopoBriteNode(
		uint	id,
		uint	xpos,
		uint	ypos,
		uint	degree
		)
	{
		this->id = id;
		this->xpos = xpos;
		this->ypos = ypos;
		this->degree = degree;

		this->edges = alloc uint[this->degree];
		assert(this->edges);

		for(uint index = 0; index < this->degree; index++) this->edges[index] = 0xFFFFFFFF;
		this->edgeSet = 0;
	}

	TopoBriteNode::~TopoBriteNode()
	{
		delete[] this->edges;
	}

	uint TopoBriteNode::SetEdge(uint edge)
	{
		assert(this->edgeSet < this->degree);
		this->edges[this->edgeSet] = edge;
		return this->edgeSet++;
	}
}
