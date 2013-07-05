/* 
 * Copyright (C) 2012 Alex Bikfalvi
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

#include "Topo.h"
#include "TopoBriteNode.h"
#include "TopoBriteEdge.h"
#include "TopoBriteRoute.h"

#define TOPO_BRITE_MODEL_WAXMAN	1

namespace SimLib
{
	class TopoBrite : public Topo
	{
	private:
		uint			model;
		char			modelName[32];
	
		uint			hs;
		uint			ls;

		// Model parameters (Waxman)
		double			alpha;
		double			beta;
		uint			nodePlacement;
		uint			growthType;
		uint			bwDist;
		uint			nodeLinks;
		__bitrate		bwMin;
		__bitrate		bwMax;

		uint			numNodes;
		uint			numEdges;
	
		// Nodes
		TopoBriteNode**		nodes;

		// Edges
		TopoBriteEdge**		edges;

		// Route
		TopoBriteRoute*		route;

	public:
		TopoBrite(const char* fileName);
		~TopoBrite();

		inline uint			Nodes() { return this->numNodes; }
		inline TopoNode*	Node(uint index) { assert(this->nodes); return this->nodes[index]; }

		inline uint			Edges() { return this->numEdges; }
		inline TopoEdge*	Edge(uint index) { assert(this->edges); return this->edges[index]; }

		inline TopoRoute*	Route() { return this->route; }

		inline __byte		Model() { return this->model; }
		inline char*		ModelName() { return this->modelName; }
	
		inline uint			Hs() { return this->hs; }
		inline uint			Ls() { return this->ls; }
		inline double		Alpha() { return this->alpha; }
		inline double		Beta() { return this->beta; }
		inline uint			NodePlacement() { return this->nodePlacement; }
		inline uint			GrowthType() { return this->growthType; }
		inline uint			BwDist() { return this->bwDist; }
		inline uint			NodeLinks() { return this->nodeLinks; }

	protected:
		void				Read(const char* fileName);
		void				GenerateRoutes();
	};
}