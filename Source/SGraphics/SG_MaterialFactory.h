#pragma once
#include "SG_Material.h"
namespace Shift
{
	class SG_MaterialFactory
	{
	public:
		static SC_Ref<SG_Material> GetCreateMaterial(const char* aPath);
		static SC_Ref<SG_Material> GetCreateMaterial(const SG_MaterialDesc& aDesc);
	private:

		static SC_HashMap<uint, SC_Ref<SG_Material>> ourCache;
	};
}

