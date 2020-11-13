#include "SClient_Precompiled.h"
#include "SClient_GameObject.h"
#include "SClient_Base.h"

#include "SG_Model.h"
#include "SG_World.h"

SClient_GameObject::SClient_GameObject()
{
}

SClient_GameObject::~SClient_GameObject()
{
	if (myModel)
	{
		if (Shift::SG_World* gfxWorld = SClient_Base::GetClient()->GetGraphicsWorld())
		{
			gfxWorld->RemoveModel(myModel);
		}
	}
}

void SClient_GameObject::TEMP_LoadFromFile(const char* /*aModelPath*/)
{
}

void SClient_GameObject::Interact()
{
	//Shift::SC_Vector3f pos = myModel->myTransform.GetPosition();
	//myGizmo.Interact(pos);
	//SetPosition(pos);
}

void SClient_GameObject::Render()
{
	//myGizmo.Render();
}

void SClient_GameObject::SetScale(const Shift::SC_Vector3f& aScale)
{
	myModel->myTransform.Scale(aScale);
}

void SClient_GameObject::SetPosition(const Shift::SC_Vector3f& aPosition)
{
	myModel->myTransform.SetPosition(aPosition);
}
