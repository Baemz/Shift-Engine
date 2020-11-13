#pragma once
namespace Shift
{
	class SG_RenderData;
	struct SG_TerrainInitData
	{
		SG_TerrainInitData()
			: myResolution(256.f, 256.f)
			, mySectorSize(1.f)
		{}
		SC_Vector2f myResolution;
		float mySectorSize;
	};

	class SG_Terrain
	{
	public:
		SG_Terrain();
		~SG_Terrain();

		bool Init(const SG_TerrainInitData& aInitData);
		void Render(const SG_RenderData& aRenderData, bool aOnlyDepth = false);

	private:
		enum { Depth, Color };

		SC_Ref<SR_Texture> myHeightMap;
		SC_Ref<SR_Texture> myNormalMap;
		SC_Ref<SR_Buffer> myVertexBuffer;
		SC_Ref<SR_Buffer> myIndexBuffer;
		SC_Ref<SR_ShaderState> myShaders[2];
	};

}