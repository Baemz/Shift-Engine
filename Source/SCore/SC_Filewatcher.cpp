#include "SCore_Precompiled.h"
#include "SC_Filewatcher.h"

namespace Shift
{
	SC_Filewatcher::SC_Filewatcher()
	{
	}

	SC_Filewatcher::~SC_Filewatcher()
	{
		if (myIsRunning)
			Stop();
	}
	void SC_Filewatcher::Watch(const std::string& aFilepath, FileWatchCallback aFunc)
	{
		if (!myIsRunning)
			return;

		std::string correctedPath(aFilepath);
		replace(correctedPath.begin(), correctedPath.end(), '/', '\\');
		correctedPath.insert(0, SC_EnginePaths::GetRelativeDataPathDblSlash());

		SC_MutexLock lock(myMutex);

		if (myFilepaths.contains(correctedPath))
			return;

		SWatchedFileData data;
		data.myTimeChanged = std::filesystem::last_write_time(correctedPath);
		data.myCallback = aFunc;
		myFilepaths[correctedPath] = data;
	}
	void SC_Filewatcher::WatchDirectory(const std::string& aDirectoryPath, DirWatchCallback aFunc, bool aCallInitially)
	{
		if (!myIsRunning)
			return;

		std::string correctedPath(aDirectoryPath);
		replace(correctedPath.begin(), correctedPath.end(), '/', '\\');

		SC_MutexLock lock(myMutex);

		if (myWatchedDirectories.contains(correctedPath))
			return;

		SWatchedDirData data;
		
		if (!aCallInitially)
			data.myTimeChanged = std::filesystem::last_write_time(correctedPath);

		data.myCallback = aFunc;
		myWatchedDirectories[correctedPath] = data;
	}
	void SC_Filewatcher::StopWatch(const std::string& aFilepath)
	{
		if (!myIsRunning)
			return;

		std::string correctedPath(aFilepath);
		replace(correctedPath.begin(), correctedPath.end(), '/', '\\');

		SC_MutexLock lock(myMutex);
		myFilepaths.erase(correctedPath);
	}
	void SC_Filewatcher::Start()
	{
		if (!ValidateWatchDirectory())
		{
			SC_ERROR_LOG("Filewatcher detected an invalid data-dir path. Disabling watcher.");
			return;
		}

		if (myIsRunning)
			Stop();

		myIsRunning = true;
		SC_ThreadProperties threadProps;
		threadProps.myName = "Filewatcher";
		threadProps.myPriority = SC_ThreadPriority_Low;
		threadProps.myAffinity = SC_ThreadTools::ourDefaultThreadAffinity;
		myThread = SC_CreateThread(threadProps, this, &SC_Filewatcher::WatcherFunc);
	}
	void SC_Filewatcher::Stop()
	{
		myIsRunning = false;
		myThread.Stop();

	}
	void SC_Filewatcher::WatcherFunc()
	{
		SC_Timer timer;
		constexpr uint FPS = 2;
		constexpr uint limit = 1000 / FPS;
		while (myIsRunning)
		{
			uint delta = static_cast<uint>(timer.GetDeltaTime() * 1000);
			if(delta < limit)
				std::this_thread::sleep_for(std::chrono::milliseconds(limit - delta));

			for (auto& file : myFilepaths)
			{
				if (!std::filesystem::exists(file.first))
				{
					file.second.myCallback(file.first, EFileEvent::Missing);
					myFilepaths.erase(file.first);
				}
			}


			for (auto& file : std::filesystem::recursive_directory_iterator(SC_EnginePaths::GetRelativeDataPathDblSlash()))
			{
				const std::string path = file.path().string();
				if (myFilepaths.contains(path))
				{
					auto lastWriteTime = std::filesystem::last_write_time(file);
					if (myFilepaths[path].myTimeChanged != lastWriteTime)
					{
						myFilepaths[path].myTimeChanged = lastWriteTime;
						myFilepaths[path].myCallback(path, EFileEvent::Modified);
						SC_LOG("Modified file reloaded. [%s]", path.c_str());
					}
				}
			}

			//for (auto& directory : myWatchedDirectories)
			//{
			//	auto lastWriteTime = std::filesystem::last_write_time(directory.first);
			//	if (directory.second.myTimeChanged != lastWriteTime)
			//	{
			//		SC_GrowingArray<std::string> files;
			//		for (auto& file : std::filesystem::recursive_directory_iterator(directory.first))
			//		{
			//			files.Add(file.path().string());
			//		}
			//		myWatchedDirectories[directory.first].myCallback(files, EFileEvent::Modified);
			//	}
			//}

			timer.Tick();
		}
	}
	bool SC_Filewatcher::ValidateWatchDirectory() const
	{
		std::filesystem::path pathObj(SC_EnginePaths::GetRelativeDataPath());
		if (std::filesystem::exists(pathObj) && std::filesystem::is_directory(pathObj))
			return true;

		return false;
	}
}