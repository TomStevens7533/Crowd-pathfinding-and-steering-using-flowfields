//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_Flowfield.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAstar.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EBFS.h"
#include "Inegrationfield\IntegrationField.h"

using namespace Elite;


int App_Flowfield::COLUMNS = 20;

int App_Flowfield::ROWS = 20;
#define TEAMSIZE 100

//Destructor
App_Flowfield::~App_Flowfield()
{
	SAFE_DELETE(m_pIntergrationfield);
	SAFE_DELETE(m_pGridGraph);

	for (auto pAgent : m_AgentVector)
	{
		SAFE_DELETE(pAgent);
	}
	m_AgentVector.clear();
}

//Functions
void App_Flowfield::Start()
{
	//Set Camera
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(39.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(73.0f, 35.0f));



	//Create Graph
	MakeGridGraph();
	m_pIntergrationfield = new InegrationField<Elite::GridTerrainNode, Elite::GraphConnection>(m_pGridGraph);
	//algo init
	//m_pCurrentAlgo = new AStar<GridTerrainNode, GraphConnection>(m_pGridGraph, m_pHeuristicFunction);


	//Setup default start path
	endPathIdx = 108;
	CalculatePath();

	//Init agents
	//init flock
	m_AgentVector.resize(TEAMSIZE);
	for (int i = 0; i < TEAMSIZE; i++)
	{
		SteeringAgent* pagent = new SteeringAgent();
		pagent->SetPosition({ Elite::randomFloat((COLUMNS * m_SizeCell)), Elite::randomFloat((ROWS * m_SizeCell)) });
		pagent->SetAutoOrient(true);
		m_AgentVector[i] = pagent;


	}

}

void App_Flowfield::Update(float deltaTime)
{
	//Get linear from flowfield
	for (int i = 0; i < TEAMSIZE; i++)
	{
		SteeringAgent* pagent = m_AgentVector[i];
		Elite::Vector2 currentFlowVec = m_pGridGraph->GetNode(PositionToIndex(pagent->GetPosition()))->GetFlowVec();
		pagent->TrimToWorld({ 0,0 }, { static_cast<float>(COLUMNS * m_SizeCell), static_cast<float>(ROWS * m_SizeCell) });
		pagent->SetLinearVelocity(currentFlowVec * pagent->GetMaxLinearSpeed());
	}

	UNREFERENCED_PARAMETER(deltaTime);



	//UPDATE/CHECK GRID HAS CHANGED
	if (m_CalcPathNeeded)
	{
		CalculatePath();
		m_CalcPathNeeded = false;
	}
}

void App_Flowfield::Render(float deltaTime) const
{
	UNREFERENCED_PARAMETER(deltaTime);

	//Render Agents
	for (size_t i = 0; i < m_AgentVector.size(); i++)
	{
		m_AgentVector[i]->Render(deltaTime);
	}


	//Render grid
	m_GraphRenderer.RenderGraph(
		m_pGridGraph,
		m_bDrawGrid,
		m_bDrawNodeNumbers,
		m_bDrawConnections,
		m_bDrawConnectionsCosts
	);

	//Render end node on top if applicable
	if (endPathIdx != invalid_node_index)
	{
		m_GraphRenderer.HighlightNodes(m_pGridGraph, { m_pGridGraph->GetNode(endPathIdx) }, END_NODE_COLOR);
	}

	//render path below if applicable
	if (m_vPath.size() > 0)
	{
		m_GraphRenderer.HighlightNodes(m_pGridGraph, m_vPath);
	}

	if (m_bDrawFlowVec) {
		float arrowLenght = m_SizeCell / 3.f;
		float arrowSidesLenght = m_SizeCell / 10.f;



		for (auto node : m_pGridGraph->GetAllActiveNodes())
		{
			if (node->GetBestCost() != static_cast<float>(TerrainType::Water)) {
				Elite::Vector2 nodePos = m_pGridGraph->GetNodeWorldPos(node);
				//DEBUGRENDERER2D->DrawDirection(nodePos, node->GetFlowVec(), 2.f, {1.f, 0.f, 0.f});
				DEBUGRENDERER2D->DrawDirection(nodePos, node->GetFlowVec(), arrowLenght, { 0.8f, 0.f, 0.2f });
				DEBUGRENDERER2D->DrawPoint((nodePos + (node->GetFlowVec() * arrowLenght)), 3.f, { 0.8f, 0.f, 0.2f }, 0.f);
				//arrow drawing
				Elite::Vector2 normalizeFlowVec = node->GetFlowVec().GetNormalized();


				Elite::Vector2 leftPerpendicularPos = { normalizeFlowVec.y * arrowSidesLenght, -normalizeFlowVec.x * arrowSidesLenght };
				leftPerpendicularPos = leftPerpendicularPos + ((nodePos + ((node->GetFlowVec()) * arrowSidesLenght)));

				Elite::Vector2 RightPerpendicularPos = { -normalizeFlowVec.y * arrowSidesLenght, normalizeFlowVec.x * arrowSidesLenght };
				RightPerpendicularPos = RightPerpendicularPos + ((nodePos + ((node->GetFlowVec()) * arrowSidesLenght)));

				DEBUGRENDERER2D->DrawSegment(nodePos + (node->GetFlowVec() * arrowLenght), leftPerpendicularPos, { 0.0f, 0.f, 1.f });

				DEBUGRENDERER2D->DrawSegment(nodePos + (node->GetFlowVec() * arrowLenght), RightPerpendicularPos, { 0.0f, 0.f, 1.f });
			}
			else {

			}

		}
	}
	

}



void App_Flowfield::MakeGridGraph()
{
	m_vPath.clear();



	if (m_pGridGraph != nullptr)
		delete m_pGridGraph;

	auto m_pTempGrid = new GridGraph<GridTerrainNode, GraphConnection>(COLUMNS, ROWS, m_SizeCell, false, false, 1.f, 1.5f);

	//if (m_pCurrentAlgo != nullptr) {
	//	m_pCurrentAlgo->ClearLists();
	//	m_pCurrentAlgo->SetNewGraph(m_pTempGrid);
	//}

	//Setup default terrain
	m_pTempGrid->GetNode(86)->SetTerrainType(TerrainType::Water);
	m_pTempGrid->GetNode(66)->SetTerrainType(TerrainType::Water);
	m_pTempGrid->GetNode(67)->SetTerrainType(TerrainType::Water);
	m_pTempGrid->GetNode(47)->SetTerrainType(TerrainType::Water);
	m_pTempGrid->RemoveConnectionsToAdjacentNodes(86);
	m_pTempGrid->RemoveConnectionsToAdjacentNodes(66);
	m_pTempGrid->RemoveConnectionsToAdjacentNodes(67);
	m_pTempGrid->RemoveConnectionsToAdjacentNodes(47);
	m_pTempGrid->GetNrOfNodes();

	m_pGridGraph = m_pTempGrid;

}

void App_Flowfield::UpdateImGui()
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		int menuWidth = 200;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: target");
		ImGui::Text("RMB: start");
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

		ImGui::Text("Flowfield");
		ImGui::Spacing();

		ImGui::Text("Middle Mouse");
		ImGui::Text("for placing end node");
		ImGui::Spacing();

		ImGui::Text("controls");
		std::string buttonText{ "" };

		ImGui::Indent();

		ImGui::Checkbox("Grid", &m_bDrawGrid);
		ImGui::Checkbox("NodeNumbers", &m_bDrawNodeNumbers);
		ImGui::Checkbox("Connections", &m_bDrawConnections);
		ImGui::Checkbox("Connections Costs", &m_bDrawConnectionsCosts);
		ImGui::Checkbox("FlowField vectors", &m_bDrawFlowVec);

		ImGui::Unindent();

		
		ImGui::Spacing();
		ImGui::Text("COLLUM: ");
		ImGui::Indent();
		if (ImGui::SliderInt("COlls:", &COLUMNS, 10, 1000)) {
			MakeGridGraph();
			m_pIntergrationfield->UpdateGraph(m_pGridGraph);
			CalculatePath();

		}
		if (ImGui::SliderInt("ROWS:", &ROWS, 10, 1000)) {
			MakeGridGraph();
			m_pIntergrationfield->UpdateGraph(m_pGridGraph);
			CalculatePath();

		}
		if (ImGui::SliderInt("CellSize:", reinterpret_cast<int*>(&m_SizeCell), 3, 50)) {
			MakeGridGraph();
			m_pIntergrationfield->UpdateGraph(m_pGridGraph);
			CalculatePath();
		}

		ImGui::Unindent();
		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}


	if (m_GraphEditor.UpdateGraph(m_pGridGraph))
	{
		m_CalcPathNeeded = true;
	}
#pragma endregion
#endif
}

int App_Flowfield::PositionToIndex(const Elite::Vector2 pos)
{
	int x = (pos.x / ((COLUMNS * m_SizeCell) / COLUMNS));
	int y = int(pos.y / ((ROWS * m_SizeCell) / ROWS)) * ROWS;

	if (x + y <= (m_pGridGraph->GetNrOfActiveNodes())) return x + y;
	return 0;
}
void App_Flowfield::CalculatePath()
{
	//Check if valid start and end node exist
	if (endPathIdx != invalid_node_index)
	{

		//m_vPath = m_pCurrentAlgo->FindPath(startNode, endNode);

		Elite::GridTerrainNode* endNode = m_pGridGraph->GetNode(endPathIdx);

		m_pIntergrationfield->CalculateIntegrationField(endNode);

		//Calculate vector field
		for (auto node : m_pGridGraph->GetAllActiveNodes()) {

			auto nodeConnections = m_pGridGraph->GetNodeConnections(node);

			float currentCost = FLT_MAX;
			Elite::GridTerrainNode* smalllestCostNeighbour = nullptr;

			for (auto connnection : m_pGridGraph->GetNodeConnections(node->GetIndex())) {
				Elite::GridTerrainNode* lookupNode;
				lookupNode = m_pGridGraph->GetNode(connnection->GetTo());

				if (lookupNode->GetBestCost() < currentCost) {
					smalllestCostNeighbour = lookupNode;
					currentCost = lookupNode->GetBestCost();
				}
			}
			if (smalllestCostNeighbour != nullptr) {
				//Calculate Index node
				int xGridDifferenceBaseNode = node->GetIndex() % (COLUMNS);
				int yGridDiffereceBaseNode = node->GetIndex() / (COLUMNS);

				//Calculate Index neighbour
				int xGridDifferenceNeighbourNode = smalllestCostNeighbour->GetIndex() % (COLUMNS);
				int yGridDiffereceNeighbourNodee = smalllestCostNeighbour->GetIndex() / (COLUMNS);

				Elite::Vector2 flowFieldVec = { static_cast<float>(xGridDifferenceNeighbourNode - xGridDifferenceBaseNode)
					,static_cast<float>(yGridDiffereceNeighbourNodee - yGridDiffereceBaseNode) };

				node->SetFlowVec(flowFieldVec.GetNormalized());
			}
		
		}



	}
	else
	{
		std::cout << "No valid end node..." << std::endl;
	}
}

void App_Flowfield::RenderUI(bool updateDone)
{
	//IMGUI
	UpdateImGui();
}
