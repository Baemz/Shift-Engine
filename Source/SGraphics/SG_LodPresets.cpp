#include "SGraphics_Precompiled.h"
#include "SG_LodPresets.h"

namespace Shift
{
	SC_GrowingArray<SG_LodPreset> SG_LodPresets::ourPresets;

	void SG_LodPresets::Create()
	{
		std::ifstream input("ShiftEngine/Modules/Graphics/LodPresets.scfg");
		SC_Json presetsJson;

		if (input.is_open())
		{
			input >> presetsJson;
			input.close();

			for (auto& preset : presetsJson["Presets"])
			{
				SG_LodPreset& lp = ourPresets.Add();
				lp.myName = preset["Name"].get<std::string>();

				uint numThresholds = (uint)preset["Percentage Thresholds"].size();
				lp.myPercentageThresholds.PreAllocate(numThresholds);
				for (uint i = 0; i < numThresholds; ++i)
					lp.myPercentageThresholds.Add(preset["Percentage Thresholds"][i].get<float>());
			}
		}
	}

	void SG_LodPresets::Destroy()
	{
		ourPresets.Reset();
	}

	const SG_LodPreset& SG_LodPresets::GetPreset(const char* aPresetId)
	{
		for (uint i = 0; i < ourPresets.Count(); ++i)
		{
			if (ourPresets[i].myName == aPresetId)
				return ourPresets[i];
		}

		return ourPresets.GetFirst();
	}
}