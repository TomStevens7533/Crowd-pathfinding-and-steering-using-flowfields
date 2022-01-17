//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_Flowfield.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAstar.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EBFS.h"
#include "Inegrationfield\IntegrationField.h"

using namespace Elite;


int App_Flowfield::m_sColls = 30;

int App_Flowfield::m_sRows = 30;

int App_Flowfield::m_sAgentAmount = 250;




//Destructor
App_Flowfield::~App_Flowfield()
{
	for (auto integrationField : m_pIntergrationfieldVec)
	{
		SAFE_DELETE(integrationField);
	}

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
	std::cout << "Flowfield rendering depends on team selection check edit menu\n";
	std::cout << "Can crash when adding more agents at runtime due to multithreading\n";

	//Set Camera
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(39.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(73.0f, 35.0f));



	//Create Graph
	MakeGridGraph();

	for (size_t i = 0; i < TEAMCOUNT; i++)
	{
		m_pIntergrationfieldVec.push_back(new InegrationField<Elite::FlowFieldNode, Elite::GraphConnection>(m_pGridGraph, i));
	}

	//Setup default start path

	for (size_t i = 0; i < (TEAMCOUNT + 1); i++)
		m_endPathVec[i] = (rand() % (m_sColls * m_sRows)) + 1;



	CalculatePath();

	//Init agents
	//init flock
	m_AgentVector.resize(m_sAgentAmount);
	int teamIdx = -1;
	Elite::Color teamColor = { 1.f, 0.f, 0.f };

	for (int i = 0; i < m_sAgentAmount; i++)
	{
		if ((i) % (m_sAgentAmount / TEAMCOUNT) == 0) {
			teamIdx++;
			teamColor = { randomFloat(0,1), randomFloat(0,1), randomFloat(0,1) };
		}

		SteeringAgent* pagent = new SteeringAgent();
		pagent->SetPosition({ Elite::randomFloat(static_cast<float>(m_sColls * m_SizeCell)), Elite::randomFloat(static_cast<float>(m_sRows * m_SizeCell)) });
		pagent->SetAutoOrient(true);

		pagent->SetBodyColor(teamColor);
		m_AgentVector[i] = pagent;


	}

}

void App_Flowfield::Update(float deltaTime)
{

	if (g_lock.try_lock()) {
		int teamIdx = -1;
		for (int i = 0; i < m_sAgentAmount; i++)
		{
			if (i == ((m_sAgentAmount / TEAMCOUNT) * (teamIdx + 1))) {
				teamIdx++;
			}

			teamIdx = Elite::Clamp(teamIdx, 0, TEAMCOUNT - 1);
			SteeringAgent* pagent = m_AgentVector[i];
			Elite::Vector2 currentFlowVec = m_pGridGraph->GetNode(PositionToIndex(pagent->GetPosition()))->GetFlowVec(teamIdx);
			pagent->TrimToWorld({ 0,0 }, { static_cast<float>(m_sColls * m_SizeCell), static_cast<float>(m_sRows * m_SizeCell) });
			pagent->SetLinearVelocity(currentFlowVec * pagent->GetMaxLinearSpeed());
			pagent->Update(deltaTime);
			pagent->SetMaxLinearSpeed(m_ImguiMaxLinearSpeed);
		}
		g_lock.unlock();

	}
	//Get linear from flowfield

	

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

	for (size_t i = 0; i < TEAMCOUNT; i++)
	{
		Elite::Color bodyColor;
		//Render end node on top if applicable
		if (m_endPathVec[i] != invalid_node_index)
		{
			bodyColor = m_AgentVector[((m_sAgentAmount / TEAMCOUNT)) * i]->GetBodyColor();
			m_GraphRenderer.HighlightNodes(m_pGridGraph, { m_pGridGraph->GetNode(m_endPathVec[i]) }, bodyColor);
		}
	}


	if (m_bDrawFlowVec) {
		float arrowLenght = m_SizeCell / 3.f;
		float arrowSidesLenght = m_SizeCell / 10.f;



		for (auto node : m_pGridGraph->GetAllActiveNodes()) //Only render debug of team 0
		{
			if (node->GetTerrainType() != TerrainType::Water) { //only render if not water
				Elite::Vector2 nodePos = m_pGridGraph->GetNodeWorldPos(node);
				//DEBUGRENDERER2D->DrawDirection(nodePos, node->GetFlowVec(), 2.f, {1.f, 0.f, 0.f});
				DEBUGRENDERER2D->DrawDirection(nodePos, node->GetFlowVec(m_ImguiTeamToEditNode), arrowLenght, { 0.8f, 0.f, 0.2f });
				DEBUGRENDERER2D->DrawPoint((nodePos + (node->GetFlowVec(m_ImguiTeamToEditNode) * arrowLenght)), 3.f, { 0.8f, 0.f, 0.2f }, 0.f);
				//arrow drawing
				Elite::Vector2 normalizeFlowVec = node->GetFlowVec(m_ImguiTeamToEditNode).GetNormalized();


				Elite::Vector2 leftPerpendicularPos = { normalizeFlowVec.y * arrowSidesLenght, -normalizeFlowVec.x * arrowSidesLenght };
				leftPerpendicularPos = leftPerpendicularPos + ((nodePos + ((node->GetFlowVec(1)) * arrowSidesLenght)));

				Elite::Vector2 RightPerpendicularPos = { -normalizeFlowVec.y * arrowSidesLenght, normalizeFlowVec.x * arrowSidesLenght };
				RightPerpendicularPos = RightPerpendicularPos + ((nodePos + ((node->GetFlowVec(m_ImguiTeamToEditNode)) * arrowSidesLenght)));

				DEBUGRENDERER2D->DrawSegment(nodePos + (node->GetFlowVec(m_ImguiTeamToEditNode) * arrowLenght), leftPerpendicularPos, { 0.0f, 0.f, 1.f });

				DEBUGRENDERER2D->DrawSegment(nodePos + (node->GetFlowVec(m_ImguiTeamToEditNode) * arrowLenght), RightPerpendicularPos, { 0.0f, 0.f, 1.f });
			}

		}
	}
	

}



void App_Flowfield::MakeGridGraph()
{
	auto m_pTempGrid = new GridGraph<FlowFieldNode, GraphConnection>(m_sColls, m_sRows, m_SizeCell, false, false, 1.f, 1.5f);

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

	auto tempElementToDelete = m_pGridGraph;

	m_pGridGraph = m_pTempGrid;

	if(tempElementToDelete != nullptr)
		delete tempElementToDelete;

	
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
		ImGui::SliderInt("Team to edit:", reinterpret_cast<int*>(&m_ImguiTeamToEditNode), 0, TEAMCOUNT - 1);
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

		if (ImGui::Button("Randomize End nodes")) {
			g_lock.lock();

			for (size_t i = 0; i < TEAMCOUNT; i++)
				m_endPathVec[i] = (rand() %  (m_sColls * m_sRows ) ) + 1;
			g_lock.unlock();
			CalculatePath();

		}
		
		ImGui::Spacing();
		ImGui::Text("COLLUM: ");
		ImGui::Indent();
		
		if (ImGui::SliderInt("CellSize:", reinterpret_cast<int*>(&m_SizeCell), 5, 50)) {
			g_lock.lock();
			MakeGridGraph();
			for (auto inegrationFieldElement : m_pIntergrationfieldVec) {
				inegrationFieldElement->UpdateGraph(m_pGridGraph);
			}
			CalculatePath();
			g_lock.unlock();

		}

		std::string ButtonText =  "Add " + std::to_string(10) + " Agents";
		if (ImGui::Button(ButtonText.c_str())) {
			g_lock.lock();
			AddAgent(10);
			g_lock.unlock();
		}

		ImGui::SliderFloat("agens Linear speed", reinterpret_cast<float*>(&m_ImguiMaxLinearSpeed), 0.f, 15.f);

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

	return Elite::Clamp(x + y, 0, (m_pGridGraph->GetNrOfActiveNodes() - 1));
}

void App_Flowfield::AddAgent(int amount)
{

	int teamIdx = -1;
	Elite::Color teamColor = { 1.f, 0.f, 0.f };
	m_sAgentAmount += amount;
	m_AgentVector.resize(m_sAgentAmount);

	for (int i = 0; i < m_sAgentAmount; i++)
	{
		if ((i) % (m_sAgentAmount / TEAMCOUNT) == 0) {
			teamIdx++;
			teamColor = { randomFloat(0,1), randomFloat(0,1), randomFloat(0,1) };
		}
		if (i < (m_sAgentAmount - amount)) {
			SteeringAgent* pagent = m_AgentVector[i];
			pagent->SetAutoOrient(true);
			pagent->SetBodyColor(teamColor);

		}
		else {
			SteeringAgent* pagent = new SteeringAgent();
			pagent->SetPosition({ Elite::randomFloat(static_cast<float>(m_sColls * m_SizeCell)), Elite::randomFloat(static_cast<float>(m_sRows * m_SizeCell)) });
			pagent->SetBodyColor(teamColor);
			pagent->SetAutoOrient(true);

			m_AgentVector[i] = pagent;
		}
	}
}

void App_Flowfield::CalculatePath()
{
	//Check if valid end node exist
	


	for (size_t i = 0; i < m_pIntergrationfieldVec.size(); i++)
	{
		if (m_endPathVec[i] == invalid_node_index) {
			std::cout << "Invalid end node index for team : " << i << std::endl;
		}

		
		Elite::FlowFieldNode* endNode = m_pGridGraph->GetNode(m_endPathVec[i]);
		m_pIntergrationfieldVec[i]->CalculateIntegrationField(endNode);
		const int integrationFieldID = m_pIntergrationfieldVec[i]->GetID();

		//Calculate vector field
		for (auto node : m_pGridGraph->GetAllActiveNodes()) {

			auto nodeConnections = m_pGridGraph->GetNodeConnections(node);

			float currentCost = FLT_MAX;
			Elite::FlowFieldNode* smalllestCostNeighbour = nullptr;

			for (auto connnection : m_pGridGraph->GetNodeConnections(node->GetIndex())) {
				Elite::FlowFieldNode* lookupNode;
				lookupNode = m_pGridGraph->GetNode(connnection->GetTo());

				if (lookupNode->GetBestCost(integrationFieldID) < currentCost) {
					smalllestCostNeighbour = lookupNode;
					currentCost = lookupNode->GetBestCost(integrationFieldID);
				}
			}
			if (smalllestCostNeighbour != nullptr) {
				//Calculate Index node
				Elite::Vector2 nodeWorldPos = m_pGridGraph->GetNodeWorldPos(node->GetIndex());
				int xGridDifferenceBaseNode = Elite::Clamp(static_cast<int>(nodeWorldPos.x / m_SizeCell), 0, m_sColls);
				int yGridDiffereceBaseNode = Elite::Clamp(static_cast<int>(nodeWorldPos.y / m_SizeCell), 0, m_sRows);

				//Calculate Index neighbour
				Elite::Vector2 neighboureWorldPos = m_pGridGraph->GetNodeWorldPos(smalllestCostNeighbour->GetIndex());
				int xGridDifferenceNeighbourNode = Elite::Clamp(static_cast<int>(neighboureWorldPos.x / m_SizeCell), 0, m_sColls);
				int yGridDiffereceNeighbourNodee = Elite::Clamp(static_cast<int>(neighboureWorldPos.y / m_SizeCell), 0, m_sRows);


				Elite::Vector2 flowFieldVec = { static_cast<float>(xGridDifferenceNeighbourNode - xGridDifferenceBaseNode)
					,static_cast<float>(yGridDiffereceNeighbourNodee - yGridDiffereceBaseNode) };

				node->SetFlowVec(flowFieldVec.GetNormalized(), integrationFieldID);
			}

		}
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

		if (closestNode != invalid_node_index) {
			m_endPathVec[m_ImguiTeamToEditNode] = closestNode;
			CalculatePath();
		}

	}
}
