#pragma once
#include "../EIGraph.h"


namespace Elite {
	template <class T_NodeType, class T_ConnectionType>
	class IAlgo
	{
	public:
		IAlgo(IGraph<T_NodeType, T_ConnectionType>* pGraph) : m_pGraph{ pGraph }
		{

		}

		virtual ~IAlgo() {};

		virtual void ClearLists() = 0;
		void SetNewGraph(IGraph<T_NodeType, T_ConnectionType>* pGraph) {
			m_pGraph = pGraph;
		}



		virtual std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode) = 0;
	protected:
		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};
}

