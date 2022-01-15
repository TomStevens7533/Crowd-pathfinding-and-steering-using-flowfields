#pragma once
#include "IAlgorithm.h"

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar : public IAlgo<T_NodeType, T_ConnectionType>
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction) : IAlgo{ pGraph }, m_HeuristicFunction{ hFunction }{
		
		}
		virtual ~AStar() override{
			m_OpenList.clear();
			m_ClosedList.clear();
		}
		inline void SetHeuristic(Heuristic newHeuristic) {
			m_HeuristicFunction = newHeuristic;
			std::cout << "Switching heuristic!!\n";
		}

		virtual void ClearLists() override {
			m_ClosedList.clear();
			m_OpenList.clear();
		}

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

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode) override;

		

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;


		Heuristic m_HeuristicFunction;

		std::vector<NodeRecord> m_OpenList;
		std::vector<NodeRecord> m_ClosedList;
	public:
		 auto GetOpenList() ->decltype(auto)  {
			return &m_OpenList;
		}
		 auto GetClosedList() ->decltype(auto) {
			return &m_ClosedList;
		}
	};



	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		auto graphCopy = m_pGraph;
		NodeRecord StartRecord;
		NodeRecord CurrentRecord;
		NodeRecord tempRecord;
		bool isContinue = false;

		m_OpenList.clear();
		m_ClosedList.clear();

		StartRecord.pNode = pStartNode;
		StartRecord.pConnection = nullptr;
		StartRecord.costSoFar = 0.f;
		StartRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);
		m_OpenList.push_back(StartRecord);

		//2
		while (!m_OpenList.empty())
		{
			float previousGcost = FLT_MAX;
			float estimatedTotalCost = FLT_MAX;
			//2A
			CurrentRecord = *std::min_element(m_OpenList.begin(), m_OpenList.end()); //derefrence iterator

			//2B
			if (CurrentRecord.pConnection != nullptr && m_pGraph->GetNode(CurrentRecord.pConnection->GetTo()) == pGoalNode)
				break;

			auto conVec = m_pGraph->GetNodeConnections(CurrentRecord.pNode->GetIndex());

			//2C
			for (auto con : conVec) //check if a con is already on the closed list
			{
				//closed List
				//open list
				//see if record is in open or closed vector
				float CurrentGcost = con->GetCost() + CurrentRecord.costSoFar;



				//2D
				auto itClosed = std::find_if(m_ClosedList.begin(), m_ClosedList.end(), [&con, graphCopy](auto& i) {
					return i.pNode == graphCopy->GetNode(con->GetTo());
					});

				if (itClosed != m_ClosedList.end()) //if connection already exists on closedList
				{
					//check if new connection is cheaper dan vorige connection
					float Newclosedgcost = (*itClosed).costSoFar;
					if (CurrentGcost >= Newclosedgcost) {
						continue;
					}
					else {
						m_ClosedList.erase(std::remove(m_ClosedList.begin(), m_ClosedList.end(), *itClosed), m_ClosedList.end());

					}
				}

				//2E
				auto itOpen = std::find_if(m_OpenList.begin(), m_OpenList.end(), [&con, graphCopy](auto i) {
					return i.pNode == graphCopy->GetNode(con->GetTo());
					});

				if (itOpen != m_OpenList.end()) //if connection already exists on the open list
				{
					float Newopengcost = (*itOpen).costSoFar;

					if (CurrentGcost >= Newopengcost) {
						continue;
					}
					else {
						m_OpenList.erase(std::remove(m_OpenList.begin(), m_OpenList.end(), *itOpen), m_OpenList.end());
					}
				}

				//2F
				NodeRecord newRecord;
				newRecord.pNode = m_pGraph->GetNode(con->GetTo());
				newRecord.pConnection = con;
				newRecord.costSoFar = con->GetCost() + CurrentRecord.costSoFar;
				newRecord.estimatedTotalCost = CurrentRecord.costSoFar + GetHeuristicCost(CurrentRecord.pNode, pGoalNode) + con->GetCost();
				m_OpenList.push_back(newRecord);

			}
			//2G
			m_OpenList.erase(std::remove_if(m_OpenList.begin(),
				m_OpenList.end(),
				[&CurrentRecord](auto i) {
					return CurrentRecord == i;
				}),
				m_OpenList.end());

			m_ClosedList.push_back(CurrentRecord);

		}


		NodeRecord pathNodeRecord = CurrentRecord;
		std::vector<T_NodeType*> path{};

		//track back
		//goal node reached
		if (pathNodeRecord.pNode != pGoalNode) {
			std::cout << "Goal node not in reach\n";
			return path;
		}

		while (pathNodeRecord.pNode != pStartNode)
		{
			path.push_back(pathNodeRecord.pNode);
			//check for connection in closedList
			auto it = std::find_if(m_ClosedList.begin(), m_ClosedList.end(), [&pathNodeRecord, &graphCopy](auto i) {
				return i.pNode == graphCopy->GetNode(pathNodeRecord.pConnection->GetFrom());

				});

			if (it != m_ClosedList.end()) {
				pathNodeRecord = *it;
			}


		}
		path.push_back(pStartNode);

		std::reverse(path.begin(), path.end());

		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}