#include "STools_Precompiled.h"
#include "STools_AssetRepository.h"

#include "SG_ModelLoader.h"

namespace Shift
{

	static constexpr const char* ourAssetRepoDir = "../Data/";

	STools_AssetRepository::STools_AssetRepository()
	{
		auto onChange = [&](const SC_GrowingArray<std::string>& aFiles, const EFileEvent& aEvent)
		{
			OnDirectoryChanged(aFiles, aEvent);
		};
		SC_EngineInterface::WatchDirectory(ourAssetRepoDir, onChange, true);
	}

	STools_AssetRepository::~STools_AssetRepository()
	{
	}

	void STools_AssetRepository::Render()
	{
		//for (auto& category : myAssets)
		//{
		//	for (auto& asset : category.second)
		//		ImGui::Text("[%s] / %s", category.first.c_str(), asset.c_str());
		//}
	}

	void STools_AssetRepository::OnDirectoryChanged(const SC_GrowingArray<std::string>& aFiles, const EFileEvent&)
	{
		myAssets.clear();
		SG_ModelLoader loader;

		for (auto& file : aFiles)
		{
			std::string fileName;
			if (loader.ReadModelName(file.c_str(), fileName))
				myAssets["MODELS"].Add(fileName);
		}
	}
}