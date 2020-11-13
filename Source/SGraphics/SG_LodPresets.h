#pragma once
namespace Shift
{

	struct SG_LodPreset
	{
		std::string myName;
		SC_GrowingArray<float> myPercentageThresholds;
	};

	class SG_LodPresets
	{
	public:
		static void Create();
		static void Destroy();

		static const SG_LodPreset& GetPreset(const char* aPresetId);

	private:
		static SC_GrowingArray<SG_LodPreset> ourPresets;
	};
}

