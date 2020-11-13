#pragma once
#include "SShared_GameObject.h"

class SClient_GameObject final : public SShared_GameObject
{
public:
	SClient_GameObject();
	~SClient_GameObject();

	void TEMP_LoadFromFile(const char* aModelPath);

	void Init();

	void Interact();
	void Render();

	void SetScale(const Shift::SC_Vector3f& aScale);
	void SetPosition(const Shift::SC_Vector3f& aPosition);

private:
	Shift::SC_Ref<Shift::SG_Model> myModel;

};

