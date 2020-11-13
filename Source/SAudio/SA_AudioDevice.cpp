#include "SAudio_Precompiled.h"
#include "SA_AudioDevice.h"

#include "AK/SoundEngine/Common/AkMemoryMgr.h"
#include "AK/SoundEngine/Common/AkModule.h"
#include "AK/SoundEngine/Common/AkSoundEngine.h"
#include "AK/SoundEngine/Common/IAkStreamMgr.h"
#include "AK/Tools/Common/AkPlatformFuncs.h"

namespace Shift
{
	SA_AudioDevice::SA_AudioDevice()
	{
	}

	SA_AudioDevice::~SA_AudioDevice()
	{
	}

	bool SA_AudioDevice::Init()
	{
		//AkMemSettings memSettings;
		//AK::MemoryMgr::GetDefaultSettings(memSettings);
		//
		//if (AK::MemoryMgr::Init(&memSettings) != AK_Success)
		//{
		//	S_ERROR_LOG("Could not create AKMemoryManager.");
		//
		//	return false;
		//}
		//
		//AkStreamMgrSettings stmSettings;
		//AK::StreamMgr::GetDefaultSettings(stmSettings);
		//if (!AK::StreamMgr::Create(stmSettings))
		//{
		//	assert(!"Could not create the Streaming Manager");
		//	return false;
		//}
		//
		//AkInitSettings initSettings;
		//AkPlatformInitSettings platformInitSettings;
		//AK::SoundEngine::GetDefaultInitSettings(initSettings);
		//AK::SoundEngine::GetDefaultPlatformInitSettings(platformInitSettings);
		//
		//if (AK::SoundEngine::Init(&initSettings, &platformInitSettings) != AK_Success)
		//{
		//	S_ERROR_LOG("Could not initialize AKSoundEngine.");
		//	return false;
		//}

		return true;
	}
}