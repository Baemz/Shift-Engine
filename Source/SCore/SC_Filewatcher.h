#pragma once
#include "SC_GrowingArray.h"
#include "SC_Future.h"
#include "SC_Mutex.h"
#include <filesystem>

namespace Shift
{
	enum EFileEvent
	{
		Modified,
		Missing
	};

	using FileWatchCallback = std::function<void(const std::string&, const EFileEvent&)>;
	using DirWatchCallback = std::function<void(const SC_GrowingArray<std::string>&, const EFileEvent&)>;

	class SC_Filewatcher
	{
		friend class SC_Engine;
	public:
		~SC_Filewatcher();

		void Watch(const std::string& aFilepath, FileWatchCallback aFunc);
		void WatchDirectory(const std::string& aDirectoryPath, DirWatchCallback aFunc, bool aCallInitially = false);
		void StopWatch(const std::string& aFilepath);

	private:
		struct SWatchedFileData
		{
			void operator=(const SWatchedFileData& aOther)
			{
				myTimeChanged = aOther.myTimeChanged;
				myCallback = aOther.myCallback;
			}

			std::filesystem::file_time_type myTimeChanged;
			FileWatchCallback myCallback;
		};
		struct SWatchedDirData
		{
			void operator=(const SWatchedDirData& aOther)
			{
				myTimeChanged = aOther.myTimeChanged;
				myCallback = aOther.myCallback;
			}

			std::filesystem::file_time_type myTimeChanged;
			DirWatchCallback myCallback;
		};

		SC_Filewatcher();
		void Start();
		void Stop();
		void WatcherFunc();
		bool ValidateWatchDirectory() const;

		std::unordered_map<std::string, SWatchedFileData> myFilepaths;
		std::unordered_map<std::string, SWatchedDirData> myWatchedDirectories;
		SC_Future<void> myThread;
		SC_Mutex myMutex;
		std::atomic_bool myIsRunning;
	};
}

