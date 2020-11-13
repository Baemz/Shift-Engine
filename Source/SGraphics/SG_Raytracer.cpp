#include "SGraphics_Precompiled.h"
#include "SG_Raytracer.h"

#include "SG_View.h"
#include "SG_World.h"
#include "SG_Mesh.h"
#include "SG_MeshInstance.h"
#include "SG_RaytracingMesh.h"

namespace Shift
{
	SG_Raytracer::SG_Raytracer(SG_World* aParentWorld)
		: myParentWorld(aParentWorld)
		, myLastPrepareFrameIndex(0)
	{
	}

	SG_Raytracer::~SG_Raytracer()
	{
	}

	void SG_Raytracer::Init()
	{
#if SR_ENABLE_RAYTRACING
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		SR_ShaderCompiler* compiler = device->GetShaderCompiler();

		SR_ShaderStateDesc desc;
		desc.myShaderByteCodes[SR_ShaderType_Raytracing] = compiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/Raytracing/TraceAmbientOcclusion.ssf", SR_ShaderType_Raytracing);
		SR_ShaderStateDesc::RTHitGroup& hg = desc.myHitGroups.Add();
		hg.myHasClosestHit = true;

		myTraceAOShader = device->CreateShaderState(desc);

		const SC_Vector2f& framebufferRes = device->GetResolution();
		myResultAOTexture.myCreateTexture = true;
		myResultAOTexture.myCreateRWTexture = true;
		myResultAOTexture.myTextureFormat = SR_Format_R32_Float;
		myResultAOTexture.Update(framebufferRes, "RT Result");
#endif
	}

	void SG_Raytracer::PrepareScene(SG_View* aView)
	{
		SG_RenderData& prepareData = aView->GetPrepareData();
		prepareData;

#if SR_ENABLE_RAYTRACING
		prepareData.myRaytracingData.myShouldUpdateScene = prepareData.myFrameIndex != myLastPrepareFrameIndex;
		myLastPrepareFrameIndex = prepareData.myFrameIndex;

		{
			SC_MutexLock lock(myMeshMutex);
			myPendingMeshes.AddN(myParentWorld->GetMeshes());
		}
#endif
	}

	void SG_Raytracer::UpdateScene(SG_View* aView)
	{
		SG_RenderData& renderData = aView->GetRenderData_Mutable();
		renderData;

#if SR_ENABLE_RAYTRACING
		if (!renderData.myRaytracingData.myShouldUpdateScene)
			return;

		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();

		SC_GrowingArray<SC_Ref<SG_MeshInstance>> meshes;
		{
			SC_MutexLock lock(myMeshMutex);
			meshes.Swap(myPendingMeshes);
		}

		renderData.myRaytracingData.myInstances.PreAllocate(meshes.Count());
		for (SG_MeshInstance* meshInstance : meshes)
		{
			SR_RaytracingInstanceData& instanceData = renderData.myRaytracingData.myInstances.Add();
			SR_GraphicsDevice::GetDevice()->SetRaytracingInstanceData(instanceData, meshInstance->myMeshTemplate->GetCreateRTMesh()->myBLAS, meshInstance->GetTransform(), 0, 0xff, 0, true, SR_RasterizerFaceCull_None);
		}
		myScene = ctx->BuildRTScene(myScene, renderData.myRaytracingData.myInstances.GetBuffer(), renderData.myRaytracingData.myInstances.Count());
#endif
	}

	void SG_Raytracer::TraceAmbientOcclusion()
	{
#if SR_ENABLE_RAYTRACING
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		const SC_Vector2f& framebufferRes = device->GetResolution();
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();

		ctx->Transition(SR_ResourceState_UnorderedAccess, myResultAOTexture.myTextureBuffer);
		ctx->BindTextureRW(myResultAOTexture.myTextureRW, 0);

		ctx->SetShaderState(myTraceAOShader);
		ctx->Dispatch({ (uint)framebufferRes.x, (uint)framebufferRes.y, 1 });
		ctx->Transition(SR_ResourceState_Common, myResultAOTexture.myTextureBuffer);
#endif
	}

	SR_BufferView* SG_Raytracer::GetScene() const
	{
		return myScene;
	}
	
	SR_Texture* SG_Raytracer::GetResultTexture() const
	{
		return myResultAOTexture.myTexture;
	}
}
