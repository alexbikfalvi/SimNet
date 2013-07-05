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
#include "TopoBriteEdge.h"

namespace SimLib
{
	TopoBriteEdge::TopoBriteEdge(
		uint		id,
		uint		node1,
		uint		node2,
		uint		entry1,
		uint		entry2,
		double		length,
		double		delay,
		__bitrate	bandwidth
		)
	{
		this->id = id;
		this->nodes[0] = node1;
		this->nodes[1] = node2;
		this->entries[0] = entry1;
		this->entries[1] = entry2;
		this->length = length;
		this->delay = delay;
		this->bandwidth = bandwidth;
	}

	TopoBriteEdge::~TopoBriteEdge()
	{
	}

	double TopoBriteEdge::Cost()
	{
		return 100000000.0 / this->bandwidth;
	}

	uint TopoBriteEdge::OtherNode(uint node)
	{
		assert((this->nodes[0] == node) || (this->nodes[1] == node));
		return (this->nodes[0] == node)?this->nodes[1]:this->nodes[0];
	}

	uint TopoBriteEdge::Entry(uint node)
	{
		assert((this->nodes[0] == node) || (this->nodes[1] == node));
		return (this->nodes[0] == node) ? this->entries[0] : this->entries[1];
	}

	uint TopoBriteEdge::Direction(uint node)
	{
		assert((this->nodes[0] == node) || (this->nodes[1] == node));
		return (this->nodes[0] == node) ? 0 : 1;
	}
}
