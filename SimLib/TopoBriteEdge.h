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

#include "TopoEdge.h"

namespace SimLib
{
	class TopoBriteEdge : public TopoEdge 
	{
	private:
		uint		id;
		uint		nodes[2];
		double		length;
		double		delay;
		__bitrate	bandwidth;
		uint		entries[2];

	public:
		TopoBriteEdge(
			uint		id,
			uint		node1,
			uint		node2,
			uint		entry1,
			uint		entry2,
			double		length,
			double		delay,
			__bitrate	bandwidth
			);
		~TopoBriteEdge();

		virtual inline uint			Id() { return this->id; }
		virtual inline uint*		Nodes() { return this->nodes; }
		virtual inline double		Length() { return this->length; }
		virtual inline double		Delay() { return this->delay; }
		virtual inline __bitrate	Bandwidth() { return this->bandwidth; }
		virtual double				Cost();
		virtual uint				OtherNode(uint node);
		virtual uint				Entry(uint node);
		virtual uint				Direction(uint node);
	};
}
