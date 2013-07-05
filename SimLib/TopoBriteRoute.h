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

#include "TopoRoute.h"
#include "TopoBriteNode.h"
#include "TopoBriteEdge.h"

namespace SimLib
{
	class TopoBriteRoute : public TopoRoute
	{
	private:
		uint			numNodes;
		uint			numEdges;

		TopoBriteEdge**	edges;
		TopoBriteNode**	nodes;

		int*			routeEdge;
		int*			routeNode;
		TopoBriteEdge**	graph;

		uint*			distance;
	
	public:
		TopoBriteRoute(
			uint			numNodes,
			uint			numEdges,
			TopoBriteNode**	nodes,
			TopoBriteEdge**	edges
			);
		~TopoBriteRoute();

		uint	NextEdge(uint node, uint dst);
		uint	NextNode(uint node, uint dst);
		uint	Distance(uint src, uint dst);

	protected:
		void	ShortestPath(uint node);
		void	ShortestPathInit(uint node, double* dist, bool* res);
	};
}
