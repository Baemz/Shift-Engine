#pragma once

namespace Shift
{
	class SG_RenderData;
	class SG_MeshInstance;
	class SG_World;
	class SG_View;
	class SG_Raytracer
	{
	public: 
		SG_Raytracer(SG_World* aParentWorld);
		~SG_Raytracer();

		void Init();
		void PrepareScene(SG_View* aView);
		void UpdateScene(SG_View* aView);

		void TraceAmbientOcclusion();

		SR_BufferView* GetScene() const;
		SR_Texture* GetResultTexture() const;

	private:
		SC_GrowingArray<SC_Ref<SG_MeshInstance>> myPendingMeshes;

		SG_World* myParentWorld;
		SC_Mutex myMeshMutex;
		SC_Ref<SR_BufferView> myScene;
		SR_TextureResource myResultAOTexture;

		SC_Ref<SR_ShaderState> myTraceAOShader;

		uint64 myLastPrepareFrameIndex;
	};
}