#pragma once

namespace Shift
{
	/*
		Converts a normalized [0 .. 1] screen space coordinate into world-space coordinate.
	*/
	SC_Vector3f STools_ScreenToWorld(const SC_Vector2f& aScreenPos, float aDepth = 0.0f);
}