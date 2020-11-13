#include "SCore_Precompiled.h"
#include "SC_EngineInterface.h"
#include "SC_Engine.h"
namespace Shift
{
	SC_Engine* SC_EngineInterface::ourEngine = nullptr;

	void SC_EngineInterface::Exit()
	{
		ourEngine->myIsRunning = false;
	}

	const SC_Vector2f SC_EngineInterface::GetResolution()
	{
		return ourEngine->myStartParams.myResolution;
	}

	void SC_EngineInterface::WatchFile(const std::string& aFilepath, FileWatchCallback aFunc)
	{
		ourEngine->myFilewatcher.Watch(aFilepath, aFunc);
	}

	void SC_EngineInterface::WatchDirectory(const std::string& aDirectoryPath, DirWatchCallback aFunc, bool aCallInitially)
	{
		ourEngine->myFilewatcher.WatchDirectory(aDirectoryPath, aFunc, aCallInitially);
	}

	void SC_EngineInterface::StopWatchFile(const std::string& aFilepath)
	{
		ourEngine->myFilewatcher.StopWatch(aFilepath);
	}

	SG_ViewManager* SC_EngineInterface::GetViewManager()
	{
		return ourEngine->myGraphicsModule.GetViewManager();
	}
}
