#pragma once
#include "SC_Module.h"

#include "SG_World.h"

namespace Shift
{
	class SG_View;
	class CGameObject;
	class SG_Camera;
	struct SG_PointLightObject;
	class SP_RigidBody;
	class SG_Pointlight;
}

/*
	Should be rewritten into the base-class for a client.
	Init should register the modules needed for this client to work.
	Eg. Graphics, Game, Physics, Audio and so on.
*/

class SClient_GameObject;
class SClient_Base : public Shift::SC_Module
{
	SC_MODULE_DECL(SClient_Base);

public:
	SClient_Base();
	~SClient_Base();

	bool Init();
	void Update() override;
	void Render() override;

	Shift::SG_View* GetView();

	Shift::SG_Camera* GetCamera();

	Shift::SG_World* GetGraphicsWorld() { return myGraphicsWorld; }

	static SClient_Base* GetClient() { return ourInstance; }

private:
	static SClient_Base* ourInstance;

	bool Load();

	Shift::SC_Ref<Shift::SG_World> myGraphicsWorld;
	Shift::SC_Ref<Shift::SG_View> myMainView;

	Shift::SC_Ref<Shift::SP_RigidBody> _rigidbdy;

	SClient_GameObject* myTestObj;

	Shift::SC_GrowingArray<Shift::SC_Ref<Shift::SG_Pointlight>> myTestLights;

	Shift::SG_Camera* myMainCamera;

	Shift::SC_Future<bool> myLoadingEvent;
};

