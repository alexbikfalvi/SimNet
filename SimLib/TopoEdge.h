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

namespace SimLib
{
	class TopoEdge
	{
	public:
		virtual uint		Id() = 0;
		virtual uint*		Nodes() = 0;
		virtual double		Cost() = 0;
		virtual double		Delay() = 0;
		virtual double		Length() = 0;
		virtual __bitrate	Bandwidth() = 0;
		virtual uint		OtherNode(uint node) = 0;
		virtual uint		Entry(uint node) = 0;
		virtual uint		Direction(uint node) = 0;
	};
}
