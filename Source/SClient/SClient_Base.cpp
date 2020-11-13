#include "SClient_Precompiled.h"
#include "SClient_Base.h"
#include "SClient_GameObject.h"

#include "SG_World.h"
#include "SG_ModelLoader.h"
#include "SG_RenderData.h"
#include "SG_Model.h"
#include "SG_MeshInstance.h"
#include "SG_Pointlight.h"

#include "SP_PhysicsDevice.h"
#include "SP_RigidBody.h"

#include "SR_GraphicsDevice.h"

// SR_0
#include "SC_InputManager.h"

SClient_Base* SClient_Base::ourInstance = nullptr;

SClient_Base::SClient_Base()
{
	assert(ourInstance == nullptr);
	ourInstance = this;
}


SClient_Base::~SClient_Base()
{
	//delete myTestObj;
	delete myMainCamera;
}

bool SClient_Base::Init()
{
	myGraphicsWorld = new Shift::SG_World();
	myMainView = myGraphicsWorld->CreateView();

	if (myGraphicsWorld->Init() == false)
		return false;

	myMainCamera = new Shift::SG_Camera();
	const Shift::SC_Vector2f resolution = Shift::SR_GraphicsDevice::GetDevice()->GetResolution();
	myMainCamera->InitAsPerspective(resolution, 75.0f, 0.01f, 5000.f);
	myMainCamera->SetPosition(Shift::SC_Vector3f(0.0f, 0.0f, 0.0f));

	//myTestObj = new SClient_GameObject();
	//myTestObj->TEMP_LoadFromFile("SponzaPBR/sponzaPBR.obj");
	//myTestObj->SetScale({ 0.01f, 0.01f, 0.01f });

	//_rigidbdy = Shift::CPhysicsDevice::GetInstance()->CreateStaticRigidBody();

	//myLoadingEvent = SC_CreateLongTask(this, &SClient_Base::Load);
	Load();

	return true;
}

void SClient_Base::Update()
{
	SC_PROFILER_FUNCTION();
	const float aDeltaTime = Shift::SC_Timer::GetGlobalDeltaTime();
	const float aTotalTime = Shift::SC_Timer::GetGlobalTotalTime();
	//myTestObj->SetPosition(_rigidbdy->GetPosition());

	Shift::SC_Matrix44 camTransform = myMainCamera->GetTransform();
	
	bool changedRotation = false;
	float speed = 1.0f;

	// SR_0
	if (SC_InputManager::IsKeyDown(SC_Key_Shift))
	{
		speed *= 3.0f;
	}
	if (SC_InputManager::IsKeyDown(SC_Key_Q))
	{
		camTransform.RotateAroundY(-aDeltaTime * 2.f);
		changedRotation = true;
	}
	if (SC_InputManager::IsKeyDown(SC_Key_E))
	{
		camTransform.RotateAroundY(aDeltaTime * 2.f);
		changedRotation = true;
	}
	if (SC_InputManager::IsKeyDown(SC_Key_R))
	{
		camTransform.RotateAroundX(-aDeltaTime * 2.f);
		changedRotation = true;
	}
	if (SC_InputManager::IsKeyDown(SC_Key_F))
	{
		camTransform.RotateAroundX(aDeltaTime * 2.f);
		changedRotation = true;
	}

	if (changedRotation)
	{
		myMainCamera->SetTransform(camTransform);
	}

	if (SC_InputManager::IsKeyDown(SC_Key_Control))
	{
		myMainCamera->Move(myMainCamera->GetUp(), -aDeltaTime * speed);
	}
	if (SC_InputManager::IsKeyDown(SC_Key_Space))
	{
		myMainCamera->Move(myMainCamera->GetUp(), aDeltaTime * speed);
	}
	
	if (SC_InputManager::IsKeyDown(SC_Key_W))
	{
		myMainCamera->Move(myMainCamera->GetForward(), aDeltaTime * speed);
	}
	//else if (SC_InputManager::WasKeyReleased(SC_Key_W))
	//{
	//	camTransform.RotateAroundY(-aDeltaTime * 2.f);
	//	changedRotation = true;
	//}
	if (SC_InputManager::IsKeyDown(SC_Key_A))
	{
		myMainCamera->Move(myMainCamera->GetRight(), -aDeltaTime * speed);
	}
	if (SC_InputManager::IsKeyDown(SC_Key_S))
	{
		myMainCamera->Move(myMainCamera->GetForward(), -aDeltaTime * speed);
	}
	if (SC_InputManager::IsKeyDown(SC_Key_D))
	{
		myMainCamera->Move(myMainCamera->GetRight(), aDeltaTime * speed);
	}

	// Will delete if approved
	//if (GetKeyState(0x10) & 0x8000)
	//{
	//	speed *= 3.0f;
	//}
	//
	//if (GetKeyState('Q') & 0x8000)
	//{
	//	camTransform.RotateAroundY(-aDeltaTime * 2.f);
	//	changedRotation = true;
	//}
	//if (GetKeyState('E') & 0x8000)
	//{
	//	camTransform.RotateAroundY(aDeltaTime * 2.f);
	//	changedRotation = true;
	//}
	//if (GetKeyState('R') & 0x8000)
	//{
	//	camTransform.RotateAroundX(-aDeltaTime * 2.f);
	//	changedRotation = true;
	//}
	//if (GetKeyState('F') & 0x8000)
	//{
	//	camTransform.RotateAroundX(aDeltaTime * 2.f);
	//	changedRotation = true;
	//}
	//if (changedRotation)
	//{
	//	myMainCamera->SetTransform(camTransform);
	//}
	//
	//if (GetKeyState(0x11) & 0x8000) // Down
	//{
	//	myMainCamera->Move(myMainCamera->GetUp(), -aDeltaTime * speed);
	//}
	//if (GetKeyState(0x20) & 0x8000) // Up
	//{
	//	myMainCamera->Move(myMainCamera->GetUp(), aDeltaTime * speed);
	//}
	//
	//if (GetKeyState('W') & 0x8000)
	//{
	//	myMainCamera->Move(myMainCamera->GetForward(), aDeltaTime * speed);
	//}
	//if (GetKeyState('A') & 0x8000)
	//{
	//	myMainCamera->Move(myMainCamera->GetRight(), -aDeltaTime * speed);
	//}
	//if (GetKeyState('S') & 0x8000)
	//{
	//	myMainCamera->Move(myMainCamera->GetForward(), -aDeltaTime * speed);
	//}
	//if (GetKeyState('D') & 0x8000)
	//{
	//	myMainCamera->Move(myMainCamera->GetRight(), aDeltaTime * speed);
	//}

	uint i = 0;
	if (!myLoadingEvent.IsActive())
	{
		for (Shift::SG_Pointlight* light : myTestLights)
		{
			float sinval = sinf(aTotalTime);
			float cosval = cosf(aTotalTime);

			Shift::SC_Vector3f pos = light->GetPosition();
			pos.x += 2 * sinval * aDeltaTime * (0.1f * (i % 16));
			pos.z += 2 * cosval * aDeltaTime * (0.1f * (i % 16));
			light->SetPosition(pos);

			i++;
		}
	}
	myGraphicsWorld->Update();
	//myTestObj->Interact();
	//myTestObj->Render();
	myMainView->SetCamera(*myMainCamera);
}

void SClient_Base::Render()
{
	// TODO: Redo this, should not be dependent on ImGui.
	// Render straight into backbuffer.
	Shift::SC_Vector2f resolution = Shift::SR_GraphicsDevice::GetDevice()->GetResolution();
	bool open = true;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::Begin("GameClient", &open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
	ImGui::Image(GetView()->GetLastFinishedFrame(), resolution);
	ImGui::End(); // Viewport
}

Shift::SG_View* SClient_Base::GetView()
{
	return myMainView;
}

Shift::SG_Camera* SClient_Base::GetCamera()
{
	return myMainCamera;
}

bool SClient_Base::Load()
{
	Shift::SG_ModelLoader loader;

	SC_GrowingArray<SC_Ref<SG_Model>> scene;
	if (loader.ImportCollection("Assets/Imported/Models/Sponza.smcf", scene))
	{
		for (SG_Model* model : scene)
		{
			if (model)
			{
				Shift::SC_Matrix44 mat = model->myTransform;
				mat.Scale({ 0.01f, 0.01f, 0.01f });
				model->SetTransform(mat);
				myGraphicsWorld->AddMeshes(model);
			}
		}
	}

	
	//Shift::SC_Ref<Shift::SG_Model> mesh2 = loader.Import2("Assets/Imported/Models/HugeIslandicCliff.smf", false);
	//if (mesh2)
	//{
	//	Shift::SC_Matrix44 mat = mesh2->myTransform;
	//	mat.Scale({ 0.01f, 0.01f, 0.01f });
	//	mat.RotateAroundY(SC_DegreesToRadians(180.f));
	//	mat.SetPosition({ 0.f, 0.f, -2.f });
	//
	//	mesh2->SetTransform(mat);
	//	myGraphicsWorld->AddMeshes(mesh2);
	//}
	//Shift::SC_Ref<Shift::SG_Model> mesh3 = loader.Import2("Assets/Imported/Models/HugeIslandicCliff.smf", false);
	//if (mesh3)
	//{
	//	Shift::SC_Matrix44 mat = mesh3->myTransform;
	//	mat.Scale({ 0.01f, 0.01f, 0.01f });
	//	mat.SetPosition({ 14.f, 0.f, 2.f });
	//	mesh3->SetTransform(mat);
	//	myGraphicsWorld->AddMeshes(mesh3);
	//}
	//Shift::SC_Ref<Shift::SG_Model> mesh4 = loader.Import2("Assets/Imported/Models/HugeIslandicCliff.smf", false);
	//if (mesh4)
	//{
	//	Shift::SC_Matrix44 mat = mesh4->myTransform;
	//	mat.Scale({ 0.01f, 0.01f, 0.01f });
	//	mat.RotateAroundY(SC_DegreesToRadians(180.f));
	//	mat.SetPosition({ 14.f, 0.f, -2.f });
	//
	//	mesh4->SetTransform(mat);
	//	myGraphicsWorld->AddMeshes(mesh4);
	//}

	for (uint i = 0; i < 0; ++i)
	{
		Shift::SG_Pointlight* light = myTestLights.Add(new Shift::SG_Pointlight());
		light->SetState(SC_LoadState::Loading);

		float randcr = float(rand() % 255 + 1) / 255.f;
		float randcg = float(rand() % 255 + 1) / 255.f;
		float randcb = float(rand() % 255 + 1) / 255.f;
		light->SetColor({ randcr, randcg, randcb });
		light->SetIntensity(10.0f);
		light->SetRange(1.5f);

		//const Shift::SC_AABB& aabb = mesh->GetBoundingBox();
		float randpx = -15 + static_cast <float>(rand()) / (static_cast <float>(RAND_MAX / (15 - -15)));
		float randpy = 0 + static_cast <float>(rand()) / (static_cast <float>(RAND_MAX / (8 - 0)));
		float randpz = -6 + static_cast <float>(rand()) / (static_cast <float>(RAND_MAX / (6 - -6)));

		light->SetPosition({ randpx, randpy, randpz });

		light->SetState(SC_LoadState::Loaded);

		myGraphicsWorld->AddLight(light);
	}

	// FOR TESTING INSTANCING
	//SC_GrowingArray<SC_Ref<SG_Model>> scene2;
	//for (uint i = 0; i < 4; ++i)
	//{
	//	for (uint j = 0; j < 4; ++j)
	//	{
	//		if (loader.ImportCollection("Assets/Imported/Models/Sponza.smcf", scene2))
	//		{
	//			for (SG_Model* model : scene2)
	//			{
	//				if (model)
	//				{
	//					Shift::SC_Matrix44 mat = model->myTransform;
	//					mat.Scale({ 0.01f, 0.01f, 0.01f });
	//					mat.SetPosition({ float(40 * i) + 40.f, 0.f, float(25 * j) });
	//					model->SetTransform(mat);
	//					myGraphicsWorld->AddMeshes(model);
	//				}
	//			}
	//		}
	//		scene2.RemoveAll();
	//	}
	//}

	return true;
}
