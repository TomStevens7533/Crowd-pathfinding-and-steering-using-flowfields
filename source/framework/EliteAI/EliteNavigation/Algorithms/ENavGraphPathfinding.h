#pragma once
#include <vector>
#include <iostream>
#include "framework/EliteMath/EMath.h"
#include "framework\EliteAI\EliteGraphs\ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"
#include "../../EliteGraphs/EliteGraphAlgorithms/EBFS.h"

namespace Elite
{
	class NavMeshPathfinding
	{
	public:
		static std::vector<Elite::Vector2> FindPath(Elite::Vector2 startPos, Elite::Vector2 endPos, Elite::NavGraph* pNavGraph, std::vector<Elite::Vector2>& debugNodePositions, std::vector<Elite::Portal>& debugPortals, Elite::Heuristic& heuristic)
		{
			//Create the path to return
			std::vector<Elite::Vector2> finalPath{};
			debugNodePositions.clear();

			//Get the start and endTriangle
			auto startTriangle = pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos);
			auto EndTriangle = pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos);

			if ((startTriangle == nullptr || EndTriangle == nullptr)) { //check if triange is not the same;
				std::cout << "same Triangle no path\n";
				return finalPath; //return if same
			}
			else if (startTriangle == EndTriangle) {
				finalPath.push_back(endPos);
				return finalPath;
			}
		
			//We have valid start/end triangles and they are not the same
			//=> Start looking for a path
			//Copy the graph
			auto GraphCopy = pNavGraph->Clone();

			
			//Create extra node for the Start Node (Agent's position
			Elite::NavGraphNode* startNode = new Elite::NavGraphNode{ GraphCopy->GetNextFreeNodeIndex(), -1, startPos };
			GraphCopy->AddNode(startNode);
			if (startTriangle != nullptr) {
				for (auto edge : startTriangle->metaData.IndexLines)
				{
					int nodeID = pNavGraph->GetNodeIdxFromLineIdx(edge);
					if (nodeID != invalid_node_index) {
						float distance = Elite::DistanceSquared(GraphCopy->GetNodePos(GraphCopy->GetNode(nodeID)), GraphCopy->GetNodePos(startNode));
						GraphCopy->AddConnection(new Elite::GraphConnection2D{ startNode->GetIndex(), nodeID, distance });
					}
				}
			}

			//Create extra node for the endNode
			Elite::NavGraphNode* EndNode = new Elite::NavGraphNode{ GraphCopy->GetNextFreeNodeIndex(), -1, endPos };
			GraphCopy->AddNode(EndNode);

			if (EndTriangle != nullptr) {
				for (auto edge : EndTriangle->metaData.IndexLines)
				{
					int nodeID = pNavGraph->GetNodeIdxFromLineIdx(edge);
					if (nodeID != invalid_node_index) {

						float distance = Elite::DistanceSquared(GraphCopy->GetNodePos(GraphCopy->GetNode(nodeID)), GraphCopy->GetNodePos(EndNode));
						GraphCopy->AddConnection(new Elite::GraphConnection2D{ EndNode->GetIndex(), nodeID, distance });
					}
				}

			}
			else {
				return finalPath;
			}

			//Run A star on new graph
			auto pathFinder = AStar<NavGraphNode, GraphConnection2D>(GraphCopy.get(), heuristic);
			auto nodes = pathFinder.FindPath(startNode, EndNode);
			//OPTIONAL BUT ADVICED: Debug Visualisation

			for (auto node : nodes)
			{
				debugNodePositions.push_back(node->GetPosition());
			}
			 
			//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			debugPortals = SSFA::FindPortals(nodes, pNavGraph->GetNavMeshPolygon());
			finalPath = SSFA::OptimizePortals(debugPortals);
			return finalPath;
		}
	};
}
