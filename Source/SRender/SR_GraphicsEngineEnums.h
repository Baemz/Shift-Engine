#pragma once
namespace Shift
{
	enum EWindowMode
	{
		Fullscreen,
		BorderlessFullscreen,
		Windowed,
	};

	enum EContextType
	{
		SContext_Graphics,
		SContext_Compute,
		SContext_Copy,

		SContext_Count,
	};

}