#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon; 
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create nodes
	for (auto it : m_pNavMeshPolygon->GetLines())
	{
		auto lineVec = GetNavMeshPolygon()->GetTrianglesFromLineIndex(it->index);
		if (lineVec.size() > 1) {


			AddNode(new Elite::NavGraphNode{ GetNextFreeNodeIndex(),it->index,(it->p2 + it->p1) / 2.f });
		}
	}
	//2. Create connections now that every node is created
	for (auto triangle : m_pNavMeshPolygon->GetTriangles()) //loop over triangles
	{
		std::vector<NavGraphNode*> validNode;
		for (auto it : triangle->metaData.IndexLines) //loop over indexLines
		{
			for (auto node : m_Nodes)
			{
				if (node->GetLineIndex() == it) {
					validNode.push_back(node);
				}
			}
		}
		if (validNode.size() == 2) {
			Elite::GraphConnection2D* con = new Elite::GraphConnection2D{ validNode[0]->GetIndex(), validNode[1]->GetIndex() };
			AddConnection(con);
		}
		else if (validNode.size() == 3) {
			Elite::GraphConnection2D* con = new Elite::GraphConnection2D{ validNode[0]->GetIndex(), validNode[1]->GetIndex() };
			Elite::GraphConnection2D* con1 = new Elite::GraphConnection2D{ validNode[1]->GetIndex(), validNode[2]->GetIndex() };
			Elite::GraphConnection2D* con2 = new Elite::GraphConnection2D{ validNode[2]->GetIndex(), validNode[0]->GetIndex() };
			AddConnection(con);
			AddConnection(con1);
			AddConnection(con2);
		}


	}
	//3. Set the connections cost to the actual distance
	SetConnectionCostsToDistance();

}

