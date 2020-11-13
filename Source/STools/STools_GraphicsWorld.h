#pragma once
#include "STools_EditorSubModule.h"
namespace Shift
{
	class SG_World;
	class STools_GraphicsWorld final : public STools_EditorSubModule
	{
		STOOLS_DECLARESUBMODULE(STools_GraphicsWorld);
	public:
		STools_GraphicsWorld() {}
		STools_GraphicsWorld(SG_World* aWorld);
		~STools_GraphicsWorld();

		void Update() override;
		void Render() override;

		const char* GetWindowName() const override { return "Graphics World"; }
	private:
		bool myEnableTerrain;
		SG_World* myWorld;
	};
}

