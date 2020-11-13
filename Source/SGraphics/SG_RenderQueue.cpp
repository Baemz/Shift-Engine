#include "SGraphics_Precompiled.h"
#include "SG_RenderQueue.h"

#include "SG_Material.h"
#include "SG_World.h"

#define DEBUG_MESH_LODS 0

namespace Shift
{
	SG_RenderQueueBase::SG_RenderQueueBase()
		: myIsPrepared(false)
	{
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();

		SR_BufferDesc instanceBufferDesc;
		instanceBufferDesc.mySize = sizeof(SVertex_ObjectInputInstance);
		instanceBufferDesc.myStructSize = sizeof(SVertex_ObjectInputInstance);
		instanceBufferDesc.myBindFlag = SR_BindFlag_VertexBuffer;
		myInstanceBuffer = device->CreateBuffer(instanceBufferDesc);
	}

	SG_RenderQueueBase::~SG_RenderQueueBase()
	{
	}

	void SG_RenderQueueBase::Prepare()
	{
		SG_RenderQueueItem* items = myQueueItems.GetBuffer();
		uint numItemsToProcess = myQueueItems.Count();
		myInstanceData.RemoveAll();

		SC_Vector4f debug_LodColors[4];
		debug_LodColors[0] = SC_Vector4f(1.f, 0.f, 0.f, 1.f);
		debug_LodColors[1] = SC_Vector4f(0.f, 1.f, 0.f, 1.f);
		debug_LodColors[2] = SC_Vector4f(0.f, 0.f, 1.f, 1.f);
		debug_LodColors[3] = SC_Vector4f(1.f, 0.f, 1.f, 1.f);


		while (numItemsToProcess)
		{
			SG_RenderQueueItem& firstItem = *items;
			uint instanceCount = 1;

			if (firstItem.myVertexBuffer != nullptr &&
				firstItem.myIndexBuffer != nullptr)
			{
				SVertex_ObjectInputInstance& firstItemData = myInstanceData.Add();
				firstItem.myTransform.Transpose();
				firstItemData.mat0 = SC_Vector4f(firstItem.myTransform.my11, firstItem.myTransform.my12, firstItem.myTransform.my13, firstItem.myTransform.my14);
				firstItemData.mat1 = SC_Vector4f(firstItem.myTransform.my21, firstItem.myTransform.my22, firstItem.myTransform.my23, firstItem.myTransform.my24);
				firstItemData.mat2 = SC_Vector4f(firstItem.myTransform.my31, firstItem.myTransform.my32, firstItem.myTransform.my33, firstItem.myTransform.my34);
				firstItemData.mat3 = SC_Vector4f(firstItem.myTransform.my41, firstItem.myTransform.my42, firstItem.myTransform.my43, firstItem.myTransform.my44);
#if DEBUG_MESH_LODS
				firstItemData.instanceColor = firstItem.myLodIndex == -1 ? SC_Vector4f(1.0f, 1.0f, 1.0f, 1.0f) : debug_LodColors[firstItem.myLodIndex];
#else
				firstItemData.instanceColor = SC_Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
#endif

				for (uint i = 1; i < numItemsToProcess; ++i)
				{
					SG_RenderQueueItem& item = items[i];
					if (item.myRasterizerState != firstItem.myRasterizerState ||
						item.myBlendState != firstItem.myBlendState ||
						item.myStencil != firstItem.myStencil ||
						item.myVertexBuffer != firstItem.myVertexBuffer ||
						item.myIndexBuffer != firstItem.myIndexBuffer ||
						item.myShader != firstItem.myShader ||
						item.myMaterial != firstItem.myMaterial ||
						item.myLodIndex != firstItem.myLodIndex ||
						instanceCount == SC_UINT16_MAX)
						break;

					SVertex_ObjectInputInstance& data = myInstanceData.Add();
					item.myTransform.Transpose();
					data.mat0 = SC_Vector4f(item.myTransform.my11, item.myTransform.my12, item.myTransform.my13, item.myTransform.my14);
					data.mat1 = SC_Vector4f(item.myTransform.my21, item.myTransform.my22, item.myTransform.my23, item.myTransform.my24);
					data.mat2 = SC_Vector4f(item.myTransform.my31, item.myTransform.my32, item.myTransform.my33, item.myTransform.my34);
					data.mat3 = SC_Vector4f(item.myTransform.my41, item.myTransform.my42, item.myTransform.my43, item.myTransform.my44);
#if DEBUG_MESH_LODS
					data.instanceColor = item.myLodIndex == -1 ? SC_Vector4f(1.0f, 1.0f, 1.0f, 1.0f) : debug_LodColors[item.myLodIndex];
#else
					data.instanceColor = SC_Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
#endif

					++instanceCount;
				}
			}

			firstItem.myNumInstances = instanceCount;
			items += instanceCount;
			numItemsToProcess -= instanceCount;
		}

		myIsPrepared = true;
	}

	void SG_RenderQueueBase::Render(const char* aTag) const
	{
		if (!myQueueItems.Count())
			return;

		SR_PROFILER_FUNCTION_TAGGED(aTag);
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();

		if (myInstanceData.Count() > 0)
		{
			ctx->UpdateBufferData(myInstanceData.GetByteSize(), myInstanceData.GetBuffer(), myInstanceBuffer);
			myInstanceData.RemoveAll();
		}

		SG_RenderQueueItem* items = myQueueItems.GetBuffer();
		uint numItemsToProcess = myQueueItems.Count();
		uint instanceIndex = 0;

		struct Constants
		{
			float myTessellationAmount;
			float myDisplacementAmount;
		} c;
		c.myTessellationAmount = SG_World::myTessellationAmount;
		c.myDisplacementAmount = SG_World::myDisplacementAmount;
		ctx->BindConstantBuffer(&c, sizeof(c), 0);

		SG_Material* prevMaterial = nullptr;

		while (numItemsToProcess)
		{
			const SG_RenderQueueItem& renderItem = *items;
			if (renderItem.myVertexBuffer != nullptr &&
				renderItem.myIndexBuffer != nullptr)
			{
				ctx->SetBlendState(renderItem.myBlendState, SC_Vector4f(1.0f, 1.0f, 1.0f, 1.0f));
				ctx->SetRasterizerState(renderItem.myRasterizerState);

				ctx->SetVertexBuffer(0, renderItem.myVertexBuffer, 0, myInstanceBuffer, (instanceIndex * sizeof(SVertex_ObjectInputInstance)));

				ctx->SetIndexBuffer(renderItem.myIndexBuffer);

				if (renderItem.myMaterial && prevMaterial != renderItem.myMaterial)
				{
					ctx->SetTopology(renderItem.myMaterial->GetTopology());
					renderItem.myMaterial->BindTextures();
					prevMaterial = renderItem.myMaterial;
				}

				ctx->SetShaderState(renderItem.myShader);
				ctx->DrawIndexedInstanced(renderItem.myNumIndices, renderItem.myNumInstances, 0, 0, 0);
			}

			items += renderItem.myNumInstances;
			instanceIndex += renderItem.myNumInstances;
			SC_ASSERT((numItemsToProcess - renderItem.myNumInstances) < 4294900000);
			numItemsToProcess -= renderItem.myNumInstances;
		}
	}

	void SG_RenderQueueBase::Clear()
	{
		myInstanceData.RemoveAll();
		myQueueItems.RemoveAll();
		myIsPrepared = false;
	}
}