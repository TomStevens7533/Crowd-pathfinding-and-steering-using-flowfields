//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
//Application
#include "EliteInterfaces/EIApp.h"
#include "projects/App_Selector.h"
//---------- Registered Applications -----------

#ifdef ActiveApp_Flowfield
#include "projects/App_Flowfield/App_Flowfield.h"
#endif 

#include <thread>
#include <mutex>
//Hotfix for genetic algorithms project
bool gRequestShutdown = false;


void AppUpdate();
void PhysicsUpdate();



bool isInit = false;
std::atomic<bool> RenderFrameComplete = false;
std::atomic<bool> UpadateFrameComplete = true;

float Elapsed;

//Application Creation
IApp* myApp = nullptr;
Elite::EImmediateUI* pImmediateUI = new Elite::EImmediateUI();
EliteFrame* pFrame = new EliteFrame();
Elite::WindowParams params;

Camera2D* pCamera = new Camera2D(params.width, params.height);
EliteWindow* pWindow = new EliteWindow();




//Main
#undef main //Undefine SDL_main as main
int main(int argc, char* argv[])
{
	std::cout << "Rendering happens on another thread\n";

	int x{}, y{};
	bool runExeWithCoordinates{ argc == 3 };

	if (runExeWithCoordinates)
	{
		x = stoi(string(argv[1]));
		y = stoi(string(argv[2]));
	}

	try
	{
		INPUTMANAGER; //for some reason this fixes memory leaks that vld reports

		//Window Creation
		ELITE_ASSERT(pWindow, "Window has not been created.");
		pWindow->CreateEWindow(params);

		if (runExeWithCoordinates)
			pWindow->SetWindowPosition(x, y);

		//Create Frame (can later be extended by creating FrameManager for MultiThreaded Rendering)
		ELITE_ASSERT(pFrame, "Frame has not been created.");
		pFrame->CreateFrame(pWindow);

		//Create a 2D Camera for debug rendering in this case
		ELITE_ASSERT(pCamera, "Camera has not been created.");
		DEBUGRENDERER2D->Initialize(pCamera);

		//Create Immediate UI 
		ELITE_ASSERT(pImmediateUI, "ImmediateUI has not been created.");
		pImmediateUI->Initialize(pWindow->GetRawWindowHandle());

		//Create Physics
		PHYSICSWORLD; //Boot

		//Start Timer
		TIMER->Start();

		std::thread tr{&AppUpdate};

#ifdef ActiveApp_Flowfield
		myApp = new App_Flowfield ();
#endif // Flowfield


		ELITE_ASSERT(myApp, "Application has not been created.");

		//Boot application
		myApp->Start();

		isInit = true;


		//Application Loop
		while (!pWindow->ShutdownRequested())
		{
			//Timer
			TIMER->Update();
			Elapsed = TIMER->GetElapsed();

			//Window procedure first, to capture all events and input received by the window
			if (!pImmediateUI->FocussedOnUI())
				pWindow->ProcedureEWindow();
			else
				pImmediateUI->EventProcessing();


			pImmediateUI->NewFrame(pWindow->GetRawWindowHandle(), Elapsed);

			pCamera->Update();
			PHYSICSWORLD->RenderDebug();
			myApp->RenderUI(RenderFrameComplete);


			//Render and Present Frame
			myApp->Render(Elapsed);
			if (RenderFrameComplete) {



				RenderFrameComplete = false;

			}
		
			pFrame->SubmitAndFlipFrame(pImmediateUI);
	

		}
		 

		tr.join();


		//Reversed Deletion
		SAFE_DELETE(myApp);
		SAFE_DELETE(pImmediateUI);
		SAFE_DELETE(pCamera);
		SAFE_DELETE(pFrame);
		SAFE_DELETE(pWindow);


		//Shutdown All Singletons
		PHYSICSWORLD->Destroy();
		DEBUGRENDERER2D->Destroy();
		TIMER->Destroy();
		INPUTMANAGER->Destroy();


	}
	catch (const Elite_Exception& e)
	{
		std::cout << e._msg << " Error: " << std::endl;
#ifdef PLATFORM_WINDOWS
		system("pause");
#endif
		return 1;
	}

	return 0;
}
void  AppUpdate(){



	while (!pWindow->ShutdownRequested())
	{

		if (isInit && !RenderFrameComplete) {

			PHYSICSWORLD->Simulate(Elapsed);

			myApp->Update(Elapsed);

			//Update (Physics, App)

			RenderFrameComplete = true;


		}
		
	}
	INPUTMANAGER->Destroy();



}