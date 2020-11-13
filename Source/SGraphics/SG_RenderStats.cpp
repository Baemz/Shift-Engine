#include "SGraphics_Precompiled.h"
#include "SG_RenderStats.h"

namespace Shift
{
	SG_RenderStats::SG_RenderStats()
	{
		Reset();
	}

	SG_RenderStats::~SG_RenderStats()
	{
	}

	void SG_RenderStats::Reset()
	{
		myDrawnTriangles = 0;
		myNumMeshes = 0;
		myNumParticles = 0;
	}
}