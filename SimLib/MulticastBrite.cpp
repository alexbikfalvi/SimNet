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
#include "MulticastBrite.h"

namespace SimLib
{
	MulticastBrite::MulticastBrite(uint numNodes, uint numEdges, uint src, std::set<uint>& dst, TopoRoute* route) : dst(dst), route(route)
	{
		this->numNodes = numNodes;
		this->numEdges = numEdges;

		this->src = src;
	
		this->mcast.resize(this->numNodes);

		this->MulticastTree();
	}

	MulticastBrite::~MulticastBrite()
	{
	}

	uint MulticastBrite::NextNode(uint node, uint dst)
	{
		return this->route->NextNode(node, dst);
	}

	uint MulticastBrite::NextEdge(uint node, uint dst)
	{
		return this->route->NextEdge(node, dst);
	}

	uint MulticastBrite::NumTreeEdges()
	{
		return this->numTreeEdges;
	}

	uint MulticastBrite::NumTreeNodes()
	{
		return this->numTreeNodes;
	}

	uint MulticastBrite::NumTreeInteriorNodes()
	{
		return this->numTreeInteriorNodes;
	}

	uint MulticastBrite::NumTreeLeafNodes()
	{
		return this->numTreeLeafNodes;
	}

	MulticastBrite::ENodeType MulticastBrite::NodeType(uint node)
	{
		return this->mcast[node];
	}

	std::list<uint>* MulticastBrite::Edges()
	{
		return &this->edges;
	}

	void MulticastBrite::MulticastTree()
	{
		// Generate the multicast tree using the route paths from "src" to each "dst"

		for(uint index = 0; index < this->numNodes; index++)
			this->mcast[index] = TREE_OTHER_NODE;

		this->mcast[src] = TREE_SOURCE_NODE;
		this->numTreeEdges = 0;
		this->numTreeInteriorNodes = 0;
		this->numTreeLeafNodes = 0;

		for(std::set<uint>::iterator dst = this->dst.begin(); dst != this->dst.end(); dst++)
		{
			// Generate the tree for each destination

			this->mcast[*dst] = TREE_LEAF_NODE;
			this->numTreeLeafNodes++;

			uint next = this->route->NextNode(*dst, src);
			this->edges.push_back(this->route->NextEdge(*dst, src));

			this->numTreeEdges++;
		
			for(; (next != src) && (this->mcast[next] == TREE_OTHER_NODE);)
			{
				this->mcast[next] = TREE_IN_NODE;
				this->numTreeInteriorNodes++;

				this->edges.push_back(this->route->NextEdge(next, src));
				this->numTreeEdges++;
			
				next = this->route->NextNode(next, src);
			}
		}

		this->numTreeNodes = this->numTreeInteriorNodes + this->numTreeLeafNodes;
	}
}
