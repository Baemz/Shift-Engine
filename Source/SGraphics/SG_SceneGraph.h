#pragma once
#include "SG_DrawSubmitStructs.h"
#include "SG_View.h"
#include "SG_Camera.h"
#include "SG_DirectionalLight.h"

namespace Shift
{
	class SG_SceneGraph
	{
		friend class CRenderer;
		friend class SG_GraphicsModule;
	public:
		SG_SceneGraph();
		~SG_SceneGraph();

		void Init();
		void Reset();

		void SubmitDrawCommand(const SG_RenderObject& aRenderObject);
		void SubmitDrawCommand(const SG_PrimitiveRenderObject& aRenderObject);
		//void SubmitDrawCommand(const SDebugRenderObject& aRenderObject);

		void SubmitLight(const SG_PointLightObject& aLightObject);
		//void SubmitLight(const SSpotLightObject& aLightObject);
		//void SubmitLight(const SAreaLightObject& aLightObject);

		void SignalFrameRendered();
		bool IsRendered();

		SG_Camera _myMainCamera;
		SG_DirectionalLight _mySun;
		SG_PrimitiveRenderObject _mySky;
		float myUpdateDeltaTime;
		uint myUpdateFramerate;

	private:
		SC_GrowingArray<SG_RenderObject> myRenderObjects;
		SC_GrowingArray<SG_PrimitiveRenderObject> myPrimitiveRenderObjects;
		//SC_GrowingArray<SDebugRenderObject> myDebugRenderObjects;

		SC_GrowingArray<SG_PointLightObject> myPointLightObjects;
		//SC_GrowingArray<SSpotLightObject> mySpotLightObjects;
		//SC_GrowingArray<SAreaLightObject> myAreaLightObjects;

		bool myIsFrameRendered : 1;
	};

}