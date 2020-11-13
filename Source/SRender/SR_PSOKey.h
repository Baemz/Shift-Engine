#pragma once

namespace Shift
{
	struct SR_PSOKey
	{
		uint myTopologyKey;
		uint myDepthStateKey;
		uint myRasterStateKey;
		uint myBlendStateKey;
		uint myRenderTargetsKey;

		SR_PSOKey()
			: myTopologyKey(SC_UINT_MAX)
			, myDepthStateKey(SC_UINT_MAX)
			, myRasterStateKey(SC_UINT_MAX)
			, myBlendStateKey(SC_UINT_MAX)
			, myRenderTargetsKey(SC_UINT_MAX)
		{}

		bool operator==(const SR_PSOKey& aOther) const
		{
			if (myTopologyKey == aOther.myTopologyKey &&
				myDepthStateKey == aOther.myDepthStateKey &&
				myRasterStateKey == aOther.myRasterStateKey &&
				myBlendStateKey == aOther.myBlendStateKey &&
				myRenderTargetsKey == aOther.myRenderTargetsKey)
				return true;

			return false;
		}
	};

	inline uint SC_Hash(const SR_PSOKey& anItem)
	{
		uint hash = 0;
		hash ^= anItem.myTopologyKey;
		hash ^= anItem.myDepthStateKey;
		hash ^= anItem.myRasterStateKey;
		hash ^= anItem.myBlendStateKey;
		hash ^= anItem.myRenderTargetsKey;

		return hash;
	}

}