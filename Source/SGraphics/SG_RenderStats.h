#pragma once
namespace Shift
{
	/*
	
		Generic Render Stats
		This class stores stats about the rendered frame.
		THIS CLASS IS NOT THREAD-SAFE!
	
	*/

	class SG_RenderStats
	{
	public:
		SG_RenderStats();
		~SG_RenderStats();

		void Reset();

		// Meshes
		uint myDrawnTriangles;
		uint myNumMeshes;

		// Effects
		uint myNumParticles;
	};

}