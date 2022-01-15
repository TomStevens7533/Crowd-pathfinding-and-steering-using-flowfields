//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_Flowfield.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAstar.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EBFS.h"
#include "Inegrationfield\IntegrationField.h"

using namespace Elite;


int App_Flowfield::m_sColls = 20;

int App_Flowfield::m_sRows = 20;

int App_Flowfield::m_sAgentAmount = 50;


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
	m_AgentVector.resize(m_sAgentAmount);
	for (int i = 0; i < m_sAgentAmount; i++)
	{
		SteeringAgent* pagent = new SteeringAgent();
		pagent->SetPosition({ Elite::randomFloat(static_cast<float>(m_sColls * m_SizeCell)), Elite::randomFloat(static_cast<float>(m_sRows * m_SizeCell)) });
		pagent->SetAutoOrient(true);
		
		m_AgentVector[i] = pagent;


	}

}

void App_Flowfield::Update(float deltaTime)
{

	g_lock.lock();
	//Get linear from flowfield
	for (int i = 0; i < m_sAgentAmount; i++)
	{
		SteeringAgent* pagent = m_AgentVector[i];
		Elite::Vector2 currentFlowVec = m_pGridGraph->GetNode(PositionToIndex(pagent->GetPosition()))->GetFlowVec();
		pagent->TrimToWorld({ 0,0 }, { static_cast<float>(m_sColls * m_SizeCell), static_cast<float>(m_sRows * m_SizeCell) });
		pagent->SetLinearVelocity(currentFlowVec * pagent->GetMaxLinearSpeed());
		pagent->Update(deltaTime);
	}
	g_lock.unlock();

	

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
			if (node->GetBestCost() != static_cast<float>(TerrainType::Water)) { //only render if not water
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

		}
	}
	

}



void App_Flowfield::MakeGridGraph()
{
	m_vPath.clear();



	if (m_pGridGraph != nullptr)
		delete m_pGridGraph;

	auto m_pTempGrid = new GridGraph<GridTerrainNode, GraphConnection>(m_sColls, m_sRows, m_SizeCell, false, false, 1.f, 1.5f);

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
		ImGui::Text("Agent size: %i", m_sAgentAmount);
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
		if (ImGui::SliderInt("COlls:", &m_sColls, 10, 1000)) {
			g_lock.lock();
			MakeGridGraph();
			m_pIntergrationfield->UpdateGraph(m_pGridGraph);
			CalculatePath();
			g_lock.unlock();

		}
		if (ImGui::SliderInt("ROWS:", &m_sRows, 10, 1000)) {
			g_lock.lock();
			MakeGridGraph();
			m_pIntergrationfield->UpdateGraph(m_pGridGraph);
			CalculatePath();
			g_lock.unlock();


		}
		if (ImGui::SliderInt("CellSize:", reinterpret_cast<int*>(&m_SizeCell), 5, 50)) {
			g_lock.lock();
			MakeGridGraph();
			m_pIntergrationfield->UpdateGraph(m_pGridGraph);
			CalculatePath();
			g_lock.unlock();

		}
		ImGui::InputInt("Agents", &m_ImguiAgentToAdd);
		std::string ButtonText = (m_ImguiAgentToAdd > 0)
			? "Add " + std::to_string(m_ImguiAgentToAdd) + " Agents" : "Remove " + std::to_string(m_ImguiAgentToAdd) + " Agents";
		if (ImGui::Button(ButtonText.c_str())) {
			g_lock.lock();
			AddAgent(m_ImguiAgentToAdd);
			g_lock.unlock();
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
	int x = (pos.x / ((m_sColls * m_SizeCell) / m_sColls));
	int y = int(pos.y / ((m_sRows * m_SizeCell) / m_sRows)) * m_sRows;

	if (x + y <= (m_pGridGraph->GetNrOfActiveNodes() - 1)) return x + y;
	return 0;
}

void App_Flowfield::AddAgent(int amount)
{
	if (amount > 0) { //addagents
		m_AgentVector.resize(m_AgentVector.size() + amount);

		//init aganst if size > 0
		for (size_t i = (m_AgentVector.size() - amount); i < m_AgentVector.size(); i++)
		{
			SteeringAgent* pagent = new SteeringAgent();
			pagent->SetPosition({ Elite::randomFloat(static_cast<float>(m_sColls * m_SizeCell)), Elite::randomFloat(static_cast<float>(m_sRows * m_SizeCell)) });
			pagent->SetAutoOrient(true);

			m_AgentVector[i] = pagent;
		}
	}
	else { //remove agents

		for (size_t i = (m_AgentVector.size() - 1); i > (m_AgentVector.size() + amount - 1); i--)
		{
			SAFE_DELETE(m_AgentVector[i]);
		}
		m_AgentVector.resize(m_AgentVector.size() + amount);
	}
	m_sAgentAmount += amount;
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
				int xGridDifferenceBaseNode = node->GetIndex() % (m_sColls);
				int yGridDiffereceBaseNode = node->GetIndex() / (m_sColls);

				//Calculate Index neighbour
				int xGridDifferenceNeighbourNode = smalllestCostNeighbour->GetIndex() % (m_sColls);
				int yGridDiffereceNeighbourNodee = smalllestCostNeighbour->GetIndex() / (m_sColls);

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


	//INPUT
	bool const middleMousePressed = INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle);
	if (middleMousePressed)
	{
		MouseData mouseData = { INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eMiddle) };
		Elite::Vector2 mousePos = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ (float)mouseData.X, (float)mouseData.Y });

		//Find closest node to click pos
		int closestNode = m_pGridGraph->GetNodeIdxAtWorldPos(mousePos);

		endPathIdx = closestNode;
		CalculatePath();
	}
}
