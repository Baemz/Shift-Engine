#pragma once

#define ENABLE_DEBUG_PRIMITIVES !IS_FINAL

#if !ENABLE_DEBUG_PRIMITIVES

#define SG_PRIMITIVE_DRAW_LINE3D(aStart, aEnd){ SC_UNUSED(aStart);SC_UNUSED(aEnd); }
#define SG_PRIMITIVE_DRAW_LINE3D_COLORED(aStart, aEnd, aColor) { SC_UNUSED(aStart);SC_UNUSED(aEnd);SC_UNUSED(aColor); }
#define SG_PRIMITIVE_DRAW_LINE3D_COLORED2(aStart, aEnd, aStartColor, aEndColor) { SC_UNUSED(aStart);SC_UNUSED(aEnd);SC_UNUSED(aStartColor);SC_UNUSED(aEndColor); }
#define SG_PRIMITIVE_DRAW_TRAINGLE3D_COLORED(aCorner1, aCorner2, aCorner3, aColor) { SC_UNUSED(aCorner1);SC_UNUSED(aCorner2);SC_UNUSED(aCorner3);SC_UNUSED(aColor); }
#define SG_PRIMITIVE_DRAW_TRAINGLE3D(aCorner1, aCorner2, aCorner3) { SC_UNUSED(aCorner1);SC_UNUSED(aCorner2);SC_UNUSED(aCorner3); }

#else

#define SG_PRIMITIVE_DRAW_LINE3D(aStart, aEnd) Shift::SG_DebugPrimitives::Get()->DrawLine3D(aStart, aEnd);
#define SG_PRIMITIVE_DRAW_LINE3D_COLORED(aStart, aEnd, aColor) Shift::SG_DebugPrimitives::Get()->DrawLine3D(aStart, aEnd, aColor);
#define SG_PRIMITIVE_DRAW_LINE3D_COLORED2(aStart, aEnd, aStartColor, aEndColor) Shift::SG_DebugPrimitives::Get()->DrawLine3D(aStart, aEnd, aStartColor, aEndColor);

#define SG_PRIMITIVE_DRAW_TRAINGLE3D_COLORED(aCorner1, aCorner2, aCorner3, aColor) Shift::SG_DebugPrimitives::Get()->DrawTriangle3D(aCorner1, aCorner2, aCorner3, aColor);
#define SG_PRIMITIVE_DRAW_TRAINGLE3D(aCorner1, aCorner2, aCorner3) SG_PRIMITIVE_DRAW_TRAINGLE3D_COLORED(aCorner1, aCorner2, aCorner3, Shift::SC_Vector4f(1.0f, 1.0f, 1.0f, 1.0f));

#define SG_PRIMITIVE_DRAW_BOX3D_COLORED(aTransform, aColor) Shift::SG_DebugPrimitives::Get()->DrawBox3D(aTransform, aColor);
#define SG_PRIMITIVE_DRAW_BOX3D(aTransform) SG_PRIMITIVE_DRAW_BOX3D_COLORED(aTransform, Shift::SC_Vector4f(1.0f, 1.0f, 1.0f, 1.0f));

namespace Shift
{
	class SG_View;
	class SR_ShaderState;
	class SR_Buffer;
	class SG_DebugPrimitives
	{
	public:

		struct SG_PrimitiveVertex
		{
			SC_Vector4f myPosition;
			SC_Vector4f myColor;
		};

		struct SG_PrimitiveVertexTextured
		{
			SC_Vector4f myPosition;
			SC_Vector4f myColor;
			SC_Vector2f myUV;
		};

	public:
		static void Create();
		static void Destroy();

		static SG_DebugPrimitives* Get() { return ourInstance; }

		void Clear();

		void DrawLine3D(const SC_Vector3f& aStartPos, const SC_Vector3f& aEndPos);
		void DrawLine3D(const SC_Vector3f& aStartPos, const SC_Vector3f& aEndPos, const SC_Vector4f& aColor);
		void DrawLine3D(const SC_Vector3f& aStartPos, const SC_Vector3f& aEndPos, const SC_Vector4f& aStartColor, const SC_Vector4f& aEndColor);

		void DrawTriangle3D(const SC_Vector3f& aCorner1, const SC_Vector3f& aCorner2, const SC_Vector3f& aCorner3, const SC_Vector4f& aColor);

		void DrawBox3D(const SC_Matrix44& aTransform, const SC_Vector4f& aColor);

		void RenderPrimitives(SG_View* aView);

	private:
		SG_DebugPrimitives();
		~SG_DebugPrimitives();

		void RenderLines(SG_View* aView);
		void RenderTriangles(SG_View* aView);

		static SG_DebugPrimitives* ourInstance;

		SC_Mutex myLinesMutex;
		SC_Mutex myTriangleMutex;
		SC_GrowingArray<SG_PrimitiveVertex> myLineVertices3D;
		SC_GrowingArray<SG_PrimitiveVertex> myTriangleVertices3D;
		SC_Ref<SR_Buffer> myLineVertexBuffer;
		SC_Ref<SR_Buffer> myTriangleVertexBuffer;
		SC_Ref<SR_ShaderState> myLineShader;
		SC_Ref<SR_ShaderState> myTriangleShader;
	};
}

#endif