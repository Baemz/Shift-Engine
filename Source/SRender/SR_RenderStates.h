#pragma once
#include "SR_GraphicsResource.h"
#include "SR_RenderStateDescs.h"
namespace Shift
{
	class SR_RasterizerState : public SR_Resource
	{
	public:
		const SR_RasterizerStateDesc myProperties;
		const uint myKey;

		SR_RasterizerState(const SR_RasterizerStateDesc& aDesc) : myProperties(aDesc), myKey(SC_Hash(aDesc)) {}
	};

	class SR_BlendState : public SR_Resource
	{
	public:
		const SR_BlendStateDesc myProperties;
		const uint myKey;

		SR_BlendState(const SR_BlendStateDesc& aDesc) : myProperties(aDesc), myKey(SC_Hash(aDesc)) {}
	};

	class SR_DepthState : public SR_Resource
	{
	public:
		const SR_DepthStateDesc myProperties;
		const uint myKey;

		SR_DepthState(const SR_DepthStateDesc& aDesc) : myProperties(aDesc), myKey(SC_Hash(aDesc)) {}
	};

	class SR_RenderTargetFormatState : public SR_Resource
	{
	public:
		const SR_RenderTargetFormats myProperties;
		const uint myKey;

		SR_RenderTargetFormatState(const SR_RenderTargetFormats& aDesc) : myProperties(aDesc), myKey(SC_Hash(aDesc)) {}
	};

	struct SR_Viewport
	{
		SR_Viewport()
			: topLeftX(0)
			, topLeftY(0)
			, width(0)
			, height(0)
			, minDepth(0)
			, maxDepth(1)
		{}

		SR_Viewport(float aWidth, float aHeight)
			: topLeftX(0)
			, topLeftY(0)
			, width(aWidth)
			, height(aHeight)
			, minDepth(0)
			, maxDepth(1)
		{}

		SR_Viewport(const SC_Vector2f& aRes)
			: topLeftX(0)
			, topLeftY(0)
			, width(aRes.x)
			, height(aRes.y)
			, minDepth(0)
			, maxDepth(1)
		{}

		bool operator==(const SR_Viewport& aOther) const
		{
			if (topLeftX == aOther.topLeftX &&
				topLeftY == aOther.topLeftY &&
				width == aOther.width &&
				height == aOther.height &&
				minDepth == aOther.minDepth &&
				maxDepth == aOther.maxDepth)
				return true;

			return false;
		}

		float topLeftX;
		float topLeftY;
		float width;
		float height;
		float minDepth;
		float maxDepth;
	};

	struct SR_ScissorRect
	{
		SR_ScissorRect()
			: left(0)
			, top(0)
			, right(0)
			, bottom(0)
		{}

		SR_ScissorRect(long aWidth, long aHeight)
			: left(0)
			, top(0)
			, right(aWidth)
			, bottom(aHeight)
		{}

		SR_ScissorRect(const SC_Vector2i& aRes)
			: left(0)
			, top(0)
			, right(aRes.x)
			, bottom(aRes.y)
		{}

		SR_ScissorRect(const SC_Vector2f& aRes)
			: left(0)
			, top(0)
			, right(static_cast<long>(aRes.x))
			, bottom(static_cast<long>(aRes.y))
		{}

		long left;
		long top;
		long right;
		long bottom;
	};
}