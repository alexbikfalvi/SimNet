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
	class Multicast
	{
	public:
		enum ENodeType
		{
			TREE_OTHER_NODE	= 0,
			TREE_SOURCE_NODE = 1,
			TREE_IN_NODE = 2,
			TREE_LEAF_NODE = 3
		};

		virtual uint		NextNode(uint node, uint dst) = 0;
		virtual uint		NextEdge(uint node, uint dst) = 0;
		virtual ENodeType	NodeType(uint node) = 0;

		virtual uint		NumTreeEdges() = 0;
		virtual uint		NumTreeNodes() = 0;
		virtual uint		NumTreeInteriorNodes() = 0;
		virtual uint		NumTreeLeafNodes() = 0;
	};
}
