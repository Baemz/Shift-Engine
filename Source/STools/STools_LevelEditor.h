#pragma once
#include "STools_EditorSubModule.h"
#include "STools_TransformGizmo.h"

#include "SG_Camera.h"

namespace Shift
{
	class SG_World;
	class SG_Light;
	class SG_Model;

	class STools_LevelEditor final : public STools_EditorSubModule
	{
		STOOLS_DECLARESUBMODULE(STools_LevelEditor);
	public:
		STools_LevelEditor();
		~STools_LevelEditor();

		void Update() override;
		void Render() override;

		bool LoadLevel(const char* aLevelFile);

		const char* GetWindowName() const override { return "Level Editor"; }

	private:
		enum ViewportDisplayMode
		{
			Lit,
			Unlit,
			Depth,
			AO,
			GbufferNormals,
			Bloom,
			Luminance
		};

		struct TEMP_Object // Should probably reflect the gameobject
		{
			TEMP_Object(SG_Camera& aCamera) : myLocator(aCamera) {}
			SC_Ref<SG_Model> myModel;
			STools_TransformGizmo myLocator;
		};
		SC_GrowingArray<TEMP_Object*> myObjects;

		SC_GrowingArray<SC_Ref<SG_Light>> myLights;

		SC_Ref<SG_World> myWorld;
		SC_Ref<SG_View> myView;
		SC_Ref<SG_View> myView2;

		SG_Camera myCamera;
		SG_Camera myCamera2;
		SC_Vector2f myRelativeCursorPos;
		ViewportDisplayMode myVPDisplayMode;
	};
}

