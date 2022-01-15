#ifndef ASTAR_APPLICATION_H
#define ASTAR_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "framework\EliteAI\EliteGraphUtilities\EGraphEditor.h"
#include "framework\EliteAI\EliteGraphUtilities\EGraphRenderer.h"
#include "framework/EliteAI/EliteGraphs/EGraphNodeTypes.h"
#include "framework/EliteAI/EliteGraphs/EliteGraphAlgorithms/IAlgorithm.h"



//-----------------------------------------------------------------
// Application
//-----------------------------------------------------------------
class App_Flowfield final : public IApp
{
public:
	//Constructor & Destructor
	App_Flowfield() = default;
	virtual ~App_Flowfield();

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;
	void RenderUI(bool updateDone) override;


private:
	//Datamembers
	const bool ALLOW_DIAGONAL_MOVEMENT = true;
	Elite::Vector2 m_StartPosition = Elite::ZeroVector2;
	Elite::Vector2 m_TargetPosition = Elite::ZeroVector2;

	//Grid datamembers
	static int COLUMNS;
	static int ROWS;
	unsigned int m_SizeCell = 15;

	//Grid contains costs
	Elite::GridGraph<Elite::GridTerrainNode, Elite::GraphConnection>* m_pGridGraph = nullptr;


	//Pathfinding datamembers
	int startPathIdx = invalid_node_index;
	int endPathIdx = invalid_node_index;
	std::vector<Elite::GridTerrainNode*> m_vPath;

	//Editor and Visualisation
	Elite::GraphEditor m_GraphEditor;
	Elite::GraphRenderer m_GraphRenderer{};

	//Debug rendering information
	bool m_bDrawGrid = true;
	bool m_bDrawNodeNumbers = false;
	bool m_bDrawConnections = false;
	bool m_bDrawConnectionsCosts = false;
	bool m_StartSelected = true;
	bool m_CalcPathNeeded = false;
	int m_SelectedHeuristic = 4;
	int m_SelectedAlgoritm = 0;


	//PathFinding
	Elite::Heuristic m_pHeuristicFunction = Elite::HeuristicFunctions::Chebyshev;
	Elite::IAlgo<Elite::GridTerrainNode, Elite::GraphConnection>* m_pCurrentAlgo = nullptr;

	//Functions
	void MakeGridGraph();
	void UpdateImGui();
	void CalculatePath();

	//C++ make the class non-copyable
	App_Flowfield(const App_Flowfield&) = delete;
	App_Flowfield& operator=(const App_Flowfield&) = delete;



};
#endif