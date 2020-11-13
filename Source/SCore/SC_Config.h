#pragma once
#include "SR_RenderEnums.h"

namespace Shift
{
	struct SC_ConfigLayout;
	class SC_Config
	{
		friend class SC_Engine;
	public:
		static void Load();
		static void Save();

		static void SetWindowWidth(const float aWidth);
		static void SetWindowHeight(const float aHeight);

		static const std::string& GetAppName();
		static const float GetWindowWidth();
		static const float GetWindowHeight();
		static const bool IsFullscreen();
		static const SR_GraphicsAPI& GetGraphicsAPI();

	private:
		static SC_ConfigLayout* ourConfig;
	};

}