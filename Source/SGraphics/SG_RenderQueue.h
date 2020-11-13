#pragma once
#include "SR_RenderStates.h"
#include "VertexDefinitions.h"

namespace Shift
{
	class SR_Buffer;
	class SG_Mesh;
	class SR_ShaderState;
	class SG_Material;
	struct SG_RenderQueueItem
	{
		SG_RenderQueueItem()
		{
			memset(this, 0, sizeof(SG_RenderQueueItem));
		}
		SG_RenderQueueItem(const SG_RenderQueueItem& aOther)
		{
			assign(aOther);
		}
		SG_RenderQueueItem& operator=(const SG_RenderQueueItem& aOther)
		{
			assign(aOther);
			return *this;
		}

		SG_Material* myMaterial;
		SR_ShaderState* myShader;

		SC_Matrix44 myTransform;

		uint8 myStencil;

		uint myNumIndices;
		uint myNumInstances;
		int myLodIndex;
		float mySortDistance;

		SR_RasterizerState* myRasterizerState;
		SR_BlendState* myBlendState;
		SR_Buffer* myVertexBuffer;
		SR_Buffer* myIndexBuffer;

		void assign(const SG_RenderQueueItem& aOther)
		{
			sizeof(SG_RenderQueueItem);
			static_assert(sizeof(SG_RenderQueueItem) == 8 * 18, "sizeof(SG_RenderQueueItem) is not (8*18)");

			__m128i* dst128 = reinterpret_cast<__m128i*>(this);
			const __m128i* src128 = reinterpret_cast<const __m128i*>(&aOther);

			_mm_storeu_si128(dst128,	 _mm_loadu_si128(src128));
			_mm_storeu_si128(dst128 + 1, _mm_loadu_si128(src128 + 1));
			_mm_storeu_si128(dst128 + 2, _mm_loadu_si128(src128 + 2));
			_mm_storeu_si128(dst128 + 3, _mm_loadu_si128(src128 + 3));
			_mm_storeu_si128(dst128 + 4, _mm_loadu_si128(src128 + 4));
			_mm_storeu_si128(dst128 + 5, _mm_loadu_si128(src128 + 5));
			_mm_storeu_si128(dst128 + 6, _mm_loadu_si128(src128 + 6));
			_mm_storeu_si128(dst128 + 7, _mm_loadu_si128(src128 + 7));
			_mm_storeu_si128(dst128 + 8, _mm_loadu_si128(src128 + 8));
		}

		friend void swap(SG_RenderQueueItem& aFirst, SG_RenderQueueItem& aSecond)
		{
			static_assert(sizeof(SG_RenderQueueItem) == 8 * 18, "sizeof(SG_RenderQueueItem) is not (8*18)");

			__m128i* first128 = reinterpret_cast<__m128i*>(&aFirst);
			__m128i* second128 = reinterpret_cast<__m128i*>(&aSecond);
			__m128i temp128;
			temp128 = _mm_loadu_si128(first128);	 _mm_storeu_si128(first128,		_mm_loadu_si128(second128));	 _mm_storeu_si128(second128,	 temp128);
			temp128 = _mm_loadu_si128(first128 + 1); _mm_storeu_si128(first128 + 1, _mm_loadu_si128(second128 + 1)); _mm_storeu_si128(second128 + 1, temp128);
			temp128 = _mm_loadu_si128(first128 + 2); _mm_storeu_si128(first128 + 2, _mm_loadu_si128(second128 + 2)); _mm_storeu_si128(second128 + 2, temp128);
			temp128 = _mm_loadu_si128(first128 + 3); _mm_storeu_si128(first128 + 3, _mm_loadu_si128(second128 + 3)); _mm_storeu_si128(second128 + 3, temp128);
			temp128 = _mm_loadu_si128(first128 + 4); _mm_storeu_si128(first128 + 4, _mm_loadu_si128(second128 + 4)); _mm_storeu_si128(second128 + 4, temp128);
			temp128 = _mm_loadu_si128(first128 + 5); _mm_storeu_si128(first128 + 5, _mm_loadu_si128(second128 + 5)); _mm_storeu_si128(second128 + 5, temp128);
			temp128 = _mm_loadu_si128(first128 + 6); _mm_storeu_si128(first128 + 6, _mm_loadu_si128(second128 + 6)); _mm_storeu_si128(second128 + 6, temp128);
			temp128 = _mm_loadu_si128(first128 + 7); _mm_storeu_si128(first128 + 7, _mm_loadu_si128(second128 + 7)); _mm_storeu_si128(second128 + 7, temp128);
			temp128 = _mm_loadu_si128(first128 + 8); _mm_storeu_si128(first128 + 8, _mm_loadu_si128(second128 + 8)); _mm_storeu_si128(second128 + 8, temp128);
		}
	};

	struct SG_SortFarFirst
	{
		void Sort(SC_GrowingArray<SG_RenderQueueItem>& aQueue) const
		{
			std::sort(aQueue.begin(), aQueue.end(), [](const SG_RenderQueueItem& aItem1, const SG_RenderQueueItem& aItem2)
				{
					if (aItem1.mySortDistance != aItem2.mySortDistance)
						return aItem1.mySortDistance > aItem2.mySortDistance;

					if (aItem1.myRasterizerState != aItem2.myRasterizerState)
						return aItem1.myRasterizerState < aItem2.myRasterizerState;

					if (aItem1.myBlendState != aItem2.myBlendState)
						return aItem1.myBlendState < aItem2.myBlendState;

					if (aItem1.myStencil != aItem2.myStencil)
						return aItem1.myStencil < aItem2.myStencil;

					if (aItem1.myShader != aItem2.myShader)
						return aItem1.myShader < aItem2.myShader;

					if (aItem1.myMaterial != aItem2.myMaterial)
						return aItem1.myMaterial < aItem2.myMaterial;

					//return aItem1.myInstanceData < aItem1.myInstanceData;
					return false;

				});
		}
	};
	struct SG_SortByState
	{
		void Sort(SC_GrowingArray<SG_RenderQueueItem>& aQueue)
		{
			std::sort(aQueue.begin(), aQueue.end(), [](const SG_RenderQueueItem& aItem1, const SG_RenderQueueItem& aItem2)
				{
					if (aItem1.myRasterizerState != aItem2.myRasterizerState)
						return aItem1.myRasterizerState < aItem2.myRasterizerState;

					if (aItem1.myBlendState != aItem2.myBlendState)
						return aItem1.myBlendState < aItem2.myBlendState;

					if (aItem1.myStencil != aItem2.myStencil)
						return aItem1.myStencil < aItem2.myStencil;

					if (aItem1.myMaterial != aItem2.myMaterial)
						return aItem1.myMaterial < aItem2.myMaterial;

					if (aItem1.myShader != aItem2.myShader)
						return aItem1.myShader < aItem2.myShader;

					if (aItem1.myVertexBuffer != aItem2.myVertexBuffer)
						return aItem1.myVertexBuffer < aItem2.myVertexBuffer;

					if (aItem1.myIndexBuffer != aItem2.myIndexBuffer)
						return aItem1.myIndexBuffer < aItem2.myIndexBuffer;

					//return aItem1.myInstanceData < aItem1.myInstanceData;

					return aItem1.mySortDistance > aItem2.mySortDistance;

				});
		}
	};

	class SG_RenderQueueBase
	{
	public:
		SG_RenderQueueBase();
		~SG_RenderQueueBase();

		void Prepare();
		void Render(const char* aTag = "RenderQueue") const;
		void Clear();

		SC_Ref<SR_Buffer> myInstanceBuffer;
		SC_GrowingArray<SG_RenderQueueItem> myQueueItems; 
		mutable SC_GrowingArray<SVertex_ObjectInputInstance> myInstanceData;
		bool myIsPrepared;
	};


	template<class Sorter>
	class SG_RenderQueue : public SG_RenderQueueBase
	{
	public:
		void Prepare()
		{
			mySorter.Sort(myQueueItems);
			SG_RenderQueueBase::Prepare();
		}
		Sorter mySorter;
	};

	using SG_RenderQueue_FarFirst = SG_RenderQueue<SG_SortFarFirst>;
	using SG_RenderQueue_State = SG_RenderQueue<SG_SortByState>;
}