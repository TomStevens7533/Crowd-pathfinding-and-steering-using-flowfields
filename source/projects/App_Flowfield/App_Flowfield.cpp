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

//Destructor
App_Flowfield::~App_Flowfield()
{
	SAFE_DELETE(m_pGridGraph);
	SAFE_DELETE(m_pIntergrationfield);
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
	startPathIdx = 44;
	endPathIdx = 108;
	CalculatePath();

}

void App_Flowfield::Update(float deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);

	//INPUT
	bool const middleMousePressed = INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle);
	if (middleMousePressed)
	{
		MouseData mouseData = { INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eMiddle) };
		Elite::Vector2 mousePos = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ (float)mouseData.X, (float)mouseData.Y });

		//Find closest node to click pos
		int closestNode = m_pGridGraph->GetNodeIdxAtWorldPos(mousePos);
		if (m_StartSelected)
		{
			startPathIdx = closestNode;
			CalculatePath();
		}
		else
		{
			endPathIdx = closestNode;
			CalculatePath();
		}
	}


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

	////Astar debug rendering
	//if (dynamic_cast<Elite::AStar<GridTerrainNode, GraphConnection>*>(m_pCurrentAlgo)) {
	//	//if Astar
	//	auto Astar = dynamic_cast<Elite::AStar<GridTerrainNode, GraphConnection>*>(m_pCurrentAlgo);

	//	auto openList = Astar->GetOpenList();
	//	auto closedList = Astar->GetClosedList();


	//	for (size_t i = 0; i < openList->size(); i++)
	//	{
	//		auto currentNode = (*openList)[i].pNode;
	//		if (currentNode != nullptr && currentNode->GetTerrainType() != TerrainType::Mud)
	//			m_GraphRenderer.RenderRectNode(m_pGridGraph->GetNodeWorldPos(currentNode->GetIndex()), "", 15.f, { 0.f,0.f,1.f, 0.2f });

	//	}

	//	for (size_t i = 0; i < closedList->size(); i++)
	//	{
	//		auto currentNode = (*closedList)[i].pNode;
	//		if (currentNode != nullptr && currentNode->GetTerrainType() != TerrainType::Mud)
	//			m_GraphRenderer.RenderRectNode(m_pGridGraph->GetNodeWorldPos(currentNode->GetIndex()), "", 15.f, { 0.7f,0.f,0.5f, 0.25f });
	//	}


	//}
	//else if (dynamic_cast<Elite::BFS<GridTerrainNode, GraphConnection>*>(m_pCurrentAlgo)) {
	//	//if Astar
	//	auto BFS = dynamic_cast<Elite::BFS<GridTerrainNode, GraphConnection>*>(m_pCurrentAlgo);

	//	auto openList = BFS->GetOpenList();
	//	auto closedList = BFS->GetClosedList();

	//	for (auto& currentNode : *openList)
	//	{
	//		if(currentNode != nullptr && currentNode->GetTerrainType() != TerrainType::Mud)
	//		m_GraphRenderer.RenderRectNode(m_pGridGraph->GetNodeWorldPos(currentNode->GetIndex()), "", 15.f, { 0.f,0.f,1.f, 0.2f });
	//	}


	//	for (auto& currentNode : *closedList)
	//	{
	//		if(currentNode.second !=  nullptr && currentNode.second->GetTerrainType() != TerrainType::Mud)
	//			m_GraphRenderer.RenderRectNode(m_pGridGraph->GetNodeWorldPos(currentNode.second->GetIndex()), "", 15.f, { 0.7f,0.f,0.5f, 0.25f });
	//	}


	//}


	//Render grid
	m_GraphRenderer.RenderGraph(
		m_pGridGraph,
		m_bDrawGrid,
		m_bDrawNodeNumbers,
		m_bDrawConnections,
		m_bDrawConnectionsCosts
	);
	//Render start node on top if applicable
	if (startPathIdx != invalid_node_index)
	{
		m_GraphRenderer.HighlightNodes(m_pGridGraph, { m_pGridGraph->GetNode(startPathIdx) }, START_NODE_COLOR);
	}

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
		ImGui::Text("controls");
		std::string buttonText{ "" };
		if (m_StartSelected)
			buttonText += "Start Node";
		else
			buttonText += "End Node";

		if (ImGui::Button(buttonText.c_str()))
		{
			m_StartSelected = !m_StartSelected;
		}

		ImGui::Checkbox("Grid", &m_bDrawGrid);
		ImGui::Checkbox("NodeNumbers", &m_bDrawNodeNumbers);
		ImGui::Checkbox("Connections", &m_bDrawConnections);
		ImGui::Checkbox("Connections Costs", &m_bDrawConnectionsCosts);

		//if(ImGui::Combo("Algorithm", &m_SelectedAlgoritm, "Astar\0BFS", 3)) {

		//	switch (m_SelectedAlgoritm)
		//	{
		//	case 0: //ASTAR
		//		if (!dynamic_cast<Elite::AStar<GridTerrainNode, GraphConnection>*>(m_pCurrentAlgo)) {
		//			delete m_pCurrentAlgo;
		//			m_pCurrentAlgo = new Elite::AStar<Elite::GridTerrainNode, Elite::GraphConnection>(m_pGridGraph, m_pHeuristicFunction);
		//		}
		//		break;
		//	case 1: //BFS
		//		if (dynamic_cast<Elite::AStar<GridTerrainNode, GraphConnection>*>(m_pCurrentAlgo)) {
		//			delete m_pCurrentAlgo;
		//			m_pCurrentAlgo = new Elite::BFS<Elite::GridTerrainNode, Elite::GraphConnection>(m_pGridGraph);
		//		}
		//		break;
		//	default:
		//		break;
		//	}
		//}

		//if (dynamic_cast<Elite::AStar<GridTerrainNode, GraphConnection>*>(m_pCurrentAlgo)) { //if astar render heuristic imgui
		//	if (ImGui::Combo("", &m_SelectedHeuristic, "Manhattan\0Euclidean\0SqrtEuclidean\0Octile\0Chebyshev", 4))
		//	{
		//		switch (m_SelectedHeuristic)
		//		{
		//		case 0:
		//			m_pHeuristicFunction = HeuristicFunctions::Manhattan;
		//			break;
		//		case 1:
		//			m_pHeuristicFunction = HeuristicFunctions::Euclidean;
		//			break;
		//		case 2:
		//			m_pHeuristicFunction = HeuristicFunctions::SqrtEuclidean;
		//			break;
		//		case 3:
		//			m_pHeuristicFunction = HeuristicFunctions::Octile;
		//			break;
		//		case 4:
		//			m_pHeuristicFunction = HeuristicFunctions::Chebyshev;
		//			break;
		//		default:
		//			m_pHeuristicFunction = HeuristicFunctions::Chebyshev;
		//			break;
		//		}

		//		//if Astar
		//		//auto Astar = dynamic_cast<Elite::AStar<GridTerrainNode, GraphConnection>*>(m_pCurrentAlgo);
		//		//Astar->SetHeuristic(m_pHeuristicFunction);


		//	}
		//}
		ImGui::Spacing();
		ImGui::Text("COLLUM: ");
		ImGui::Indent();
		if (ImGui::SliderInt("COlls:", &COLUMNS,  0, 1000 ))
			MakeGridGraph();
		if (ImGui::SliderInt("ROWS:", &ROWS,  0, 1000 ))
			MakeGridGraph();

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

void App_Flowfield::CalculatePath()
{
	//Check if valid start and end node exist
	if (startPathIdx != invalid_node_index
		&& endPathIdx != invalid_node_index
		&& startPathIdx != endPathIdx)
	{
		//BFS Pathfinding
		auto startNode = m_pGridGraph->GetNode(startPathIdx);
		auto endNode = m_pGridGraph->GetNode(endPathIdx);

		//m_vPath = m_pCurrentAlgo->FindPath(startNode, endNode);

		m_pIntergrationfield->CalculateIntegrationField(endNode);

		std::cout << "New Path Calculated node count: " << m_vPath.size() <<  std::endl;
	}
	else
	{
		std::cout << "No valid start and end node..." << std::endl;
		m_vPath.clear();
	}
}

void App_Flowfield::RenderUI(bool updateDone)
{
	//IMGUI
	UpdateImGui();
}
