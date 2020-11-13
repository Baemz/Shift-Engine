#include "SGraphics_Precompiled.h"
#include "SG_SceneGraph.h"

namespace Shift
{
	SG_SceneGraph::SG_SceneGraph()
		: myIsFrameRendered(false)
	{
	}


	SG_SceneGraph::~SG_SceneGraph()
	{
	}
	void SG_SceneGraph::Init()
	{
	}
	void SG_SceneGraph::Reset()
	{
		myIsFrameRendered = false;

		myRenderObjects.RemoveAll();
		myPrimitiveRenderObjects.RemoveAll();

		myPointLightObjects.RemoveAll();

	}
	void SG_SceneGraph::SubmitDrawCommand(const SG_RenderObject& aRenderObject)
	{
		myRenderObjects.Add(aRenderObject);
	}
	void SG_SceneGraph::SubmitDrawCommand(const SG_PrimitiveRenderObject& aRenderObject)
	{
		myPrimitiveRenderObjects.Add(aRenderObject);
	}
	//void CSceneGraph::SubmitDrawCommand(const SDebugRenderObject& aRenderObject)
	//{
	//}
	void SG_SceneGraph::SubmitLight(const SG_PointLightObject& aLightObject)
	{
		myPointLightObjects.Add(aLightObject);
	}
	//void CSceneGraph::SubmitLight(const SSpotLightObject& aLightObject)
	//{
	//}
	//void CSceneGraph::SubmitLight(const SAreaLightObject& aLightObject)
	//{
	//}
	void SG_SceneGraph::SignalFrameRendered()
	{
		myIsFrameRendered = true;
	}
	bool SG_SceneGraph::IsRendered()
	{
		return myIsFrameRendered;
	}
}