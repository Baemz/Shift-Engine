#pragma once
#include "SC_Filewatcher.h"

namespace Shift
{
	class SG_SceneGraph;
	class SG_ViewManager;
	class SC_Engine;

	class SC_EngineInterface
	{
	public:
		static void Exit();
		static const SC_Vector2f GetResolution();
		
		static void WatchFile(const std::string& aFilepath, FileWatchCallback aFunc);
		static void WatchDirectory(const std::string& aDirectoryPath, DirWatchCallback aFunc, bool aCallInitially = false);
		static void StopWatchFile(const std::string& aFilepath);
		static SG_ViewManager* GetViewManager();

	private:
		friend class SC_Engine;
		static SC_Engine* ourEngine;

	};
}