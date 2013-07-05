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
#include "TopoBriteRoute.h"

#define MAX_DIST		((double)1.7E+308)

namespace SimLib
{
	TopoBriteRoute::TopoBriteRoute(
		uint			numNodes,
		uint			numEdges,
		TopoBriteNode**	nodes,
		TopoBriteEdge**	edges
		)
	{
		assert(nodes);
		assert(edges);
		assert(numNodes);
		assert(numEdges);

		this->numNodes = numNodes;
		this->numEdges = numEdges;

		this->nodes = nodes;
		this->edges = edges;

		// Create the route (nodes and edges)
		this->routeNode = alloc int[this->numNodes*this->numNodes];
		assert(this->routeNode);

		this->routeEdge = alloc int[this->numNodes*this->numNodes];
		assert(this->routeNode);

		// Init the route
		for(uint index = 0; index < this->numNodes*this->numNodes; index++)
		{
			this->routeNode[index] = -1;
			this->routeEdge[index] = -1;
		}

		// Distance vector
		this->distance = alloc uint[this->numNodes*this->numNodes];
		assert(this->numNodes);

		// Calculate the shortest path tree for each node
		for(uint index = 0; index < this->numNodes; index++)
			this->ShortestPath(index);
	}

	TopoBriteRoute::~TopoBriteRoute()
	{
		delete[] this->routeNode;
		delete[] this->routeEdge;
		delete[] this->distance;
	}

	void TopoBriteRoute::ShortestPath(uint node)
	{
		// The shortest path from every node in the graph to "node"

		// Allocate the distance array
		double* dist = alloc double[this->numNodes];
		assert(dist);

		// Allocate the results array
		bool* res = alloc bool[this->numNodes];

		// Init
		this->ShortestPathInit(node, dist, res);
	
		for(uint numRes = this->numNodes; numRes;)
		{
			// Search for the node with the lowest distance that was not previously selected
			uint bestNode;
			double bestDist = MAX_DIST;
			for(uint index = 0; index < this->numNodes; index++)
			{
				if(res[index])
					if(dist[index] < bestDist)
					{
						bestDist = dist[index];
						bestNode = index;
					}
			}

			// If the minimum distance is infinite, stop
			if(MAX_DIST == bestDist) break;

			// Set the node with the minimum distance as selected
			res[bestNode] = 0;

			// For each node connected to the best node
			for(uint index = 0; index < this->nodes[bestNode]->Degree(); index++)
			{
				// Get the node (the node that is at the other end of the link)
				uint neighbor = this->edges[this->nodes[bestNode]->Edge(index)]->OtherNode(bestNode);

				double cost = dist[bestNode] + this->edges[this->nodes[bestNode]->Edge(index)]->Cost();

				if(cost < dist[neighbor])
				{
					dist[neighbor] = cost;
					this->distance[node*this->numNodes+neighbor] = this->distance[node*this->numNodes+bestNode] + 1;

					// The next node from "neighbor" on the route to the "node" is "bestNode"
					this->routeNode[node * this->numNodes + neighbor] = bestNode;
					this->routeEdge[node * this->numNodes + neighbor] = this->nodes[bestNode]->Edge(index);
				}
			}
		
		}
	
		// Delete the distance array
		delete[] dist;

		// Delete the results array
		delete[] res;
	}

	void TopoBriteRoute::ShortestPathInit(uint node, double* dist, bool* res)
	{
		for(uint index = 0; index < this->numNodes; index++)
		{
			this->distance[node*this->numNodes+index] = 0xFFFFFFFF;
			dist[index] = MAX_DIST;
			res[index] = 1;
		}
		this->distance[node*this->numNodes+node] = 0;
		dist[node] = 0;
	}

	uint TopoBriteRoute::NextEdge(uint node, uint dst)
	{
		assert(this->routeNode[dst * this->numNodes + node] >= 0);
		return this->routeEdge[dst * this->numNodes + node];
	}

	uint TopoBriteRoute::NextNode(uint node, uint dst)
	{
		assert(this->routeNode[dst * this->numNodes + node] >= 0);
		return this->routeNode[dst * this->numNodes + node];
	}

	uint TopoBriteRoute::Distance(uint src, uint dst)
	{
		return this->distance[src*this->numNodes+dst];
	}
}
