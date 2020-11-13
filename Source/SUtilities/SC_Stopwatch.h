#pragma once
#include <chrono>
#include <string>
#include <iostream>

namespace Shift
{
	class SC_Stopwatch
	{
	public:
		SC_Stopwatch(const std::string& aName = "Stopwatch", const bool aPrint = false)
			: myShouldPrint(false)
			, myDidStop(false)
		{
			Start(aName, aPrint);
		}
		~SC_Stopwatch()
		{
			if (myDidStop == false)
			{
				Stop();
			}
		}

		inline void Start(const std::string& aName = "Stopwatch", const bool aPrint = false)
		{
			myName = aName;
			myDidStop = false;
			myShouldPrint = aPrint;
			myStartTime = std::chrono::high_resolution_clock::now();
		}

		inline float Stop()
		{
			const auto newTick = std::chrono::high_resolution_clock::now();
			const std::chrono::microseconds microseconds = std::chrono::duration_cast<std::chrono::microseconds>(newTick - myStartTime);
			myDidStop = true;
			float milliseconds = microseconds.count() * 0.001f;
			if (myShouldPrint)
			{
				std::cout << "[" << myName << "] ended after " << milliseconds << " ms" << std::endl;
			}
			return milliseconds;
		}
	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> myStartTime;
		std::string myName;
		bool myDidStop;
		bool myShouldPrint;
	};
}