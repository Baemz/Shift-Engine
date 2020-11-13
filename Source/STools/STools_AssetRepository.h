#pragma once
#include "STools_AssetTypes.h"
#include "STools_EditorSubModule.h"

namespace Shift
{
	class STools_AssetRepository final : public STools_EditorSubModule
	{
		STOOLS_DECLARESUBMODULE(STools_AssetRepository);
	public:
		STools_AssetRepository();
		~STools_AssetRepository();

		void Render() override;

		const char* GetWindowName() const override { return "Asset Repo"; }
	private:

		void OnDirectoryChanged(const SC_GrowingArray<std::string>& aFiles, const EFileEvent&);

		std::unordered_map<std::string, SC_GrowingArray<std::string>> myAssets;
	};
}
