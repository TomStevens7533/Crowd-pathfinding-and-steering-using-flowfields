#pragma once


template <class T_NodeType, class T_ConnectionType>
class InegrationField
{
public:
	InegrationField(IGraph<T_NodeType, T_ConnectionType>* pGraph);

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

	void CalculateIntegrationField(int targetX, int targetY);



private:
	std::vector<NodeRecord> m_OpenList;
	std::vector<NodeRecord> m_ClosedList;

	IGraph<T_NodeType, T_ConnectionType>* m_pGraph;

};