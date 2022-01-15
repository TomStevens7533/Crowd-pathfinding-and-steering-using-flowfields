#pragma once
#include "framework/EliteAI/EliteGraphs/EIGraph.h"
#include "framework/EliteAI/EliteGraphs/EGraphNodeTypes.h"
#include <memory>
#include <vector>
#include <numeric>
#include <algorithm>

template <class T_NodeType, class T_ConnectionType>
class InegrationField
{
public:
	InegrationField(Elite::IGraph<T_NodeType, T_ConnectionType>* pGraph);

	// stores the optimal connection to a node and its total costs related to the start and end node of the path
	struct NodeRecord
	{
		T_NodeType* pNode = nullptr;
		T_ConnectionType* pConnection = nullptr;
		float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
		float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

		bool operator==(const NodeRecord& other) const
		{
			return pNode == other.pNode
				&& pConnection == other.pConnection
				&& costSoFar == other.costSoFar
				&& estimatedTotalCost == other.estimatedTotalCost;
		};

		bool operator<(const NodeRecord& other) const
		{
			return estimatedTotalCost < other.estimatedTotalCost;
		};
	};

	void CalculateIntegrationField(T_NodeType* goalNode);
	void UpdateGraph(Elite::IGraph<T_NodeType, T_ConnectionType>* newGraph);

private:
	void ResetField();

private:
	std::vector<T_NodeType*> m_OpenList;
	std::vector<T_NodeType*> m_ClosedList;

	Elite::IGraph<T_NodeType, T_ConnectionType>* m_pGraph;

};

template <class T_NodeType, class T_ConnectionType>
void InegrationField<T_NodeType, T_ConnectionType>::UpdateGraph(Elite::IGraph<T_NodeType, T_ConnectionType>* newGraph)
{
	m_pGraph = newGraph;
}

template <class T_NodeType, class T_ConnectionType>
void InegrationField<T_NodeType, T_ConnectionType>::ResetField()
{
	for (auto node : m_pGraph->GetAllActiveNodes())
	{
		//Set cost to max cost equal to water node
		node->SetBestCost(static_cast<int>(TerrainType::Water));
	}
}

template <class T_NodeType, class T_ConnectionType>
void InegrationField<T_NodeType, T_ConnectionType>::CalculateIntegrationField(T_NodeType* goalNode)
{
	//Reset graphCopy to max cost;
	ResetField();
	//Set goal node cost to 0 and add it to the open list
	goalNode->SetBestCost(0);
	m_OpenList.push_back(goalNode);

	//Algorithm continues until open list is empty
	while (m_OpenList.size() > 0)
	{
		//Get the next node in the open list
		T_NodeType* currentLookUpNode = m_OpenList.front();
		m_OpenList.erase(m_OpenList.begin(), m_OpenList.begin() + 1);
		
		for (auto connnection : m_pGraph->GetNodeConnections(currentLookUpNode->GetIndex()))
		{
			//Get the neighbour Node
			T_NodeType* neighbourNode = m_pGraph->GetNode(connnection->GetTo());
			//Calculate new Node Cost based upon the curent node and the connection cost of going to the nextnode;
			int CalculatedCost = static_cast<int>(m_pGraph->GetNode(connnection->GetTo())->GetTerrainType()); //GetCostOfTerrainType
			CalculatedCost += (m_pGraph->GetConnection(connnection->GetFrom(), connnection->GetTo())->GetCost() * currentLookUpNode->GetBestCost());

			//if shorter cost has been found add to the open list
			if (CalculatedCost < neighbourNode->GetBestCost()) {
				//Set new cost value to neighbour nodes
				neighbourNode->SetBestCost(static_cast<float>(CalculatedCost));
				//add node to openlist
				auto openListIt = std::find(m_OpenList.begin(), m_OpenList.end(), neighbourNode);
				if (openListIt == m_OpenList.end()) {
					m_OpenList.push_back(neighbourNode);
				}
			}
		}
	}
}

template <class T_NodeType, class T_ConnectionType>
InegrationField<T_NodeType, T_ConnectionType>::InegrationField(Elite::IGraph<T_NodeType, T_ConnectionType>* pGraph)
{
	m_pGraph = pGraph;
}
