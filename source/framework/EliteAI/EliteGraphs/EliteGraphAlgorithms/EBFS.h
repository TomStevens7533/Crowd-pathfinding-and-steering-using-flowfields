#pragma once
#include "IAlgorithm.h"

namespace Elite 
{
	template <class T_NodeType, class T_ConnectionType>
	class BFS : public IAlgo<T_NodeType, T_ConnectionType>
	{
	public:
		BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode) override;

		virtual void ClearLists() override {
			m_ClosedList.clear();
			m_OpenList.clear();
		}

	private:
		std::list<T_NodeType*> m_OpenList; //Frontier expanding edge
		std::map<T_NodeType*, T_NodeType*> m_ClosedList; //Already checked nodes
	public:
		auto GetOpenList() ->decltype(&m_OpenList) {
			return &m_OpenList;
		}
		auto GetClosedList() ->decltype(&m_ClosedList) {
			return &m_ClosedList;
		}
	};

	template <class T_NodeType, class T_ConnectionType>
	BFS<T_NodeType, T_ConnectionType>::BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: IAlgo(pGraph)
	{

	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> BFS<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode)
	{

		m_ClosedList.clear();
		m_OpenList.clear();

		std::queue<T_NodeType*> openList;
		openList.push(pStartNode);


		while (!openList.empty())
		{
			//neem nieuwste node van front op queue
			T_NodeType* currentNode = openList.front();
			openList.pop();
			
			if (currentNode == pDestinationNode) {
				//if node is destination node break
				break;
			}
			//loop over currentnode connections;
			for (auto con : m_pGraph->GetNodeConnections(currentNode))
			{

				T_NodeType* nextNode = m_pGraph->GetNode(con->GetTo());
				if (m_ClosedList.find(nextNode) == m_ClosedList.end()) {
					//is not in closed List;
					openList.push(nextNode);
					m_ClosedList[nextNode] = currentNode;
				}
			}


		}
		std::vector<T_NodeType*> path;
		T_NodeType* currentNode = pDestinationNode;

		while (currentNode != pStartNode)
		{
			path.push_back(currentNode);
			currentNode = m_ClosedList[currentNode]; //assign currentNode to previous node

		}
		path.push_back(pStartNode);
		std::reverse(path.begin(), path.end());


		while (!openList.empty()) {
			m_OpenList.push_back(openList.front());
			openList.pop();
		}


		//we found goal node backtrack
		return path;
	}
}

