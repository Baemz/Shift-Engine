#pragma once
#include "SG_Camera.h"


namespace Shift
{
	class SG_View;

	class SG_Voxelizer
	{
	public: 
		SG_Voxelizer();
		~SG_Voxelizer();

		void Voxelize(SG_View* aView);
		void RelightVoxelScene(SG_View* aView);

		void SetVoxelConstants();
		SR_Texture* GetVoxelScene() const { return myVoxelLit; }

		void DebugRender();

		static constexpr uint ourNumVoxelCascades = 3;
		static constexpr uint ourVoxelSceneResolution = 128;
		static constexpr float ourVoxelSize = 0.125f;

	private:
		void GenerateMipChain();

		SC_Ref<SR_ShaderState> myRelightShader;
		SC_Ref<SR_ShaderState> myGenerateMipsShader;
		SC_Ref<SR_Texture> myVoxelColors;
		SC_Ref<SR_Texture> myVoxelNormals;
		SC_Ref<SR_Texture> myVoxelLit;
		SC_Ref<SR_Texture> myVoxelColorsRW;
		SC_Ref<SR_Texture> myVoxelNormalsRW;
		SC_Ref<SR_Texture> myVoxelLitRW;
	};
}
