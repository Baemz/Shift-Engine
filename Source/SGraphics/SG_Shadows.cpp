#include "SGraphics_Precompiled.h"
#include "SG_Shadows.h"
#include "SG_View.h"
#include "SG_Terrain.h"
#include "SG_World.h"
#include "SG_RenderData.h"
#include "SG_Model.h"
#include "SG_Material.h"
#include "SG_CommonRenderStates.h"
#include "SG_ShadowConstants.h"

#include "SR_ViewConstants.h"

namespace Shift
{
	bool SG_Shadows::lockCamera = (false);
	bool SG_Shadows::wasLocked = (false);
	float SG_Shadows::ourPCFFilterScaleOverride = -1.0f;

	static float locFade = 0.08f;
	static bool locDebugDrawCameraFrustum = false;
	static bool locDebugDrawCascadeFrustums = false;
	static bool locUseCenter = false;

	struct Circle
	{
		SC_Vector2f myCenter;
		float myRadius;

		bool operator<(const Circle& aOther) const
		{
			return myRadius < aOther.myRadius;
		}
		bool operator==(const Circle& aOther) const
		{
			return myRadius == aOther.myRadius;
		}
	};

	static const SC_Vector4f locCascadeColors[SG_Shadows::ourNumShadowCascades] =
	{
		SC_Vector4f(1.0f, 0.0f, 0.0f),
		SC_Vector4f(0.0f, 0.0f, 1.0f),
		SC_Vector4f(0.0f, 1.0f, 0.0f),
		SC_Vector4f(1.0f, 1.0f, 0.0f)
	};

	void locDebugDrawFrustum(const SC_Vector3f* aFrustumPoints, const SC_Vector4f& aColor)
	{
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(aFrustumPoints[0], aFrustumPoints[1], aColor);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(aFrustumPoints[1], aFrustumPoints[3], aColor);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(aFrustumPoints[3], aFrustumPoints[2], aColor);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(aFrustumPoints[2], aFrustumPoints[0], aColor);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(aFrustumPoints[0], aFrustumPoints[4], aColor);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(aFrustumPoints[4], aFrustumPoints[5], aColor);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(aFrustumPoints[5], aFrustumPoints[1], aColor);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(aFrustumPoints[1], aFrustumPoints[5], aColor);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(aFrustumPoints[5], aFrustumPoints[7], aColor);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(aFrustumPoints[7], aFrustumPoints[3], aColor);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(aFrustumPoints[3], aFrustumPoints[7], aColor);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(aFrustumPoints[7], aFrustumPoints[6], aColor);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(aFrustumPoints[6], aFrustumPoints[2], aColor);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(aFrustumPoints[2], aFrustumPoints[6], aColor);
		SG_PRIMITIVE_DRAW_LINE3D_COLORED(aFrustumPoints[6], aFrustumPoints[4], aColor);
	}

	static void locGetMinBoundingSphereFrom2Points(const SC_Vector2f* someCoordinates, SC_GrowingArray<Circle>& someCircles)
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				if (i != j)
				{
					// Create a minimum circle containing point i and j
					const SC_Vector2f& pi = someCoordinates[i];
					const SC_Vector2f& pj = someCoordinates[j];

					Circle c;
					c.myCenter = (pi + pj) * 0.5f;
					c.myRadius = (pi - pj).Length() * 0.5f;

					// Does it contain all the points
					bool enclosing = true;
					const float distSquared = c.myRadius * c.myRadius;
					for (int k = 0; k < 4; ++k)
					{
						if (k != i && k != j)
						{
							const SC_Vector2f& pk = someCoordinates[k];
							if ((pk - c.myCenter).Length2() > (distSquared + 0.0001f))
							{
								enclosing = false;
								break;
							}
						}
					}
					// Add circle to list if it is enclosing
					if (enclosing)
					{
						someCircles.Add(c);
					}
				}
			}
		}
	}
	
	static bool IsPerpendicular(const SC_Vector2f& aC1, const SC_Vector2f& aC2, const SC_Vector2f& aC3)
	{
		float deltax1 = fabsf(aC2.x - aC1.x);
		float deltax2 = fabsf(aC3.x - aC2.x);
		float deltay1 = fabsf(aC2.y - aC1.y);
		float deltay2 = fabsf(aC3.y - aC2.y);
		float e = 0.0001f;
		if (deltax1 <= e && deltay2 <= e)
			return false;
		else if (deltax2 <= e && deltay1 <= e)
			return false;
		else if (deltax1 <= e)
			return true;
		else if (deltax2 <= e)
			return true;
		else if (deltay1 <= e)
			return true;
		else if (deltay2 <= e)
			return true;
		return false;
	}

	static void GetCircleFrom3Points(const SC_Vector2f& aC1, const SC_Vector2f& aC2, const SC_Vector2f& aC3, Circle& aCircle)
	{
		// Reset circle
		aCircle.myCenter = { 0.0f, 0.0f };
		aCircle.myRadius = 0.0f;

		SC_Vector2f p1;
		SC_Vector2f p2;
		SC_Vector2f p3;
		if (!IsPerpendicular(aC1, aC2, aC3)) { p1 = aC1; p2 = aC2; p3 = aC3; }
		else if (!IsPerpendicular(aC1, aC3, aC2)) { p1 = aC1; p2 = aC3; p3 = aC2; }
		else if (!IsPerpendicular(aC2, aC1, aC3)) { p1 = aC2; p2 = aC1; p3 = aC3; }
		else if (!IsPerpendicular(aC2, aC3, aC1)) { p1 = aC2; p2 = aC3; p3 = aC1; }
		else if (!IsPerpendicular(aC3, aC1, aC2)) { p1 = aC3; p2 = aC1; p3 = aC2; }
		else if (!IsPerpendicular(aC3, aC2, aC1)) { p1 = aC3; p2 = aC2; p3 = aC1; }
		else
		{
			return;
		}

		float deltax1 = p2.x - p1.x;
		float deltax2 = p3.x - p2.x;
		float deltay1 = p2.y - p1.y;
		float deltay2 = p3.y - p2.y;
		float e = 0.00001f;
		if (fabsf(deltax1) <= e && fabsf(deltay2) <= e)
		{
			aCircle.myCenter.x = (p2.x + p3.x) * 0.5f;
			aCircle.myCenter.y = (p1.y + p2.y) * 0.5f;
			aCircle.myRadius = (aCircle.myCenter - p1).Length();
			return;
		}
		else if (fabsf(deltax2) <= e && fabsf(deltay1) <= e)
		{
			aCircle.myCenter.x = (p1.x + p2.x) * 0.5f;
			aCircle.myCenter.y = (p2.y + p3.y) * 0.5f;
			aCircle.myRadius = (aCircle.myCenter - p1).Length();
			return;
		}

		float ma = (deltay1) / (deltax1);
		float mb = (deltay2) / (deltax2);
		if (fabsf(mb - ma) <= e)
		{
			return;
		}

		aCircle.myCenter.x = (ma * mb * (p1.y - p3.y) + mb * (p1.x + p2.x) - ma * (p2.x + p3.x)) /
			(2.0f * (mb - ma));
		aCircle.myCenter.y = (-1.0f / ma) * (aCircle.myCenter.x - (p1.x + p2.x) * 0.5f) + (p1.y + p2.y) * 0.5f;
		aCircle.myRadius = (aCircle.myCenter - p1).Length();
	}

	static void locGetMinBoundingSphereFrom3Points(const SC_Vector2f* someCoordinates, SC_GrowingArray<Circle>& someCircles)
	{
		// Try the 4 combinations
		Circle c;
		GetCircleFrom3Points(someCoordinates[0], someCoordinates[1], someCoordinates[2], c);
		if (c.myRadius > 0.0f && ((someCoordinates[3] - c.myCenter).Length() < c.myRadius + 0.0001f))
			someCircles.Add(c);

		GetCircleFrom3Points(someCoordinates[1], someCoordinates[2], someCoordinates[3], c);
		if (c.myRadius > 0.0f && ((someCoordinates[0] - c.myCenter).Length() < c.myRadius + 0.0001f))
			someCircles.Add(c);

		GetCircleFrom3Points(someCoordinates[2], someCoordinates[3], someCoordinates[0], c);
		if (c.myRadius > 0.0f && ((someCoordinates[1] - c.myCenter).Length() < c.myRadius + 0.0001f))
			someCircles.Add(c);

		GetCircleFrom3Points(someCoordinates[3], someCoordinates[0], someCoordinates[1], c);
		if (c.myRadius > 0.0f && ((someCoordinates[2] - c.myCenter).Length() < c.myRadius + 0.0001f))
			someCircles.Add(c);
	}

	SG_Shadows::SG_Shadows()
		: myShadowCastDistance(100.0f)
		, myPCFFilterScale(0.85f)
	{
	}

	SG_Shadows::~SG_Shadows()
	{
	}

	void SG_Shadows::Init()
	{
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		myRandomNoiseTexture = device->GetCreateTexture("ShiftEngine/Textures/randomrotate.png");

		SetupCSM();
	}

	void SG_Shadows::SetupCSM()
	{
		myCSMArray.Init(ourNumShadowCascades, ourCSMResolution);
	}

	void SG_Shadows::PrepareShadowConstants(SG_ShadowConstants& aConstants, const SG_RenderData& aRenderData) const
	{
		for (uint i = 0; i < SG_Shadows::ourNumShadowCascades; ++i)
		{
			aConstants.myShadowWorldToClip[i] = GetShadowProjection(aRenderData.myShadowLightCameras[i], i);
		}
		aConstants.myShadowSampleRotation = SC_Vector4f(1.0f, 0.0, 0.0, 1.0);
		aConstants.myPCFFilterOffset = GetPCFFilterOffset();
	}

	void SG_Shadows::PrepareView(SG_View* aView)
	{
		SG_RenderData& prepareData = aView->GetPrepareData();

		prepareData.mySun.SetDirection(-(aView->myWorld->mySky->GetToSunDirection()));

		CalculateCSMViewProjections(prepareData);

		for (uint i = 0; i < SG_Shadows::ourNumShadowCascades; ++i)
			prepareData.myShadowLightCameras[i] = GetCSMCamera(i);
	}

	void SG_Shadows::RenderShadows(SG_View* aView)
	{
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();

		SR_Viewport viewport;
		viewport.topLeftX = 0;
		viewport.topLeftY = 0;
		viewport.width = ourCSMResolution;
		viewport.height = ourCSMResolution;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.0f;

		SR_ScissorRect scissor;
		scissor.left = 0;
		scissor.top = 0;
		scissor.right = long(ourCSMResolution);
		scissor.bottom = long(ourCSMResolution);

		ctx->SetScissorRect(scissor);
		ctx->SetViewport(viewport);
		ctx->SetDepthState(SG_CommonRenderStates::GetInstance()->myDepthGreaterEqualWriteState, 0);

		ctx->SetTopology(SR_Topology_TriangleList);

		ctx->Transition(SR_ResourceState_DepthWrite, myCSMArray.myTextureBuffer);

		for (int i = ourNumShadowCascades - 1; i >= 0; --i)
		{
			std::string tag("Shadow Cascade ");
			tag += std::to_string(i);
			SR_PROFILER_FUNCTION_TAGGED(tag.c_str());
			RenderCascade(i, aView);
		}

		const SG_Camera& camera = aView->GetRenderData().myMainCamera;
		const SC_Matrix44& view = camera.GetView();
		const SC_Matrix44& projection = camera.GetProjection();
		const SC_Matrix44& inverseView = camera.GetInverseView();
		const SC_Matrix44& inverseProjection = camera.GetInverseProjection();

		SR_ViewConstants constants;
		constants.myWorldToClip = view * projection;
		constants.myWorldToCamera = view;
		constants.myClipToWorld = inverseProjection * inverseView;
		constants.myClipToCamera = inverseProjection;
		constants.myCameraToClip = projection;
		constants.myCameraToWorld = inverseView;
		constants.myCameraPosition = camera.GetPosition();
		constants.myCameraFov = camera.GetFovInRadians();
		constants.myCameraNear = camera.GetNear();
		constants.myCameraFar = camera.GetFar();
		constants.myResolution = SR_GraphicsDevice::GetDevice()->GetResolution();
		constants.myFrameIndex = (uint)aView->GetRenderData().myFrameIndex;
		ctx->BindConstantBufferRef(&constants, sizeof(constants), SR_ConstantBufferRef::SR_ConstantBufferRef_ViewConstants);
		ctx->Transition(SR_ResourceState(SR_ResourceState_NonPixelSRV | SR_ResourceState_PixelSRV), myCSMArray.myTextureBuffer);
	}

	SR_Texture* SG_Shadows::GetSunShadowMap() const
	{
		return myCSMArray.myTexture;
	}
	
	SR_Texture* SG_Shadows::GetSunShadowMapSlice(uint aMip) const
	{
		return myCSMArray.myTextureSlices[aMip];
	}

	SR_Texture* SG_Shadows::GetShadowNoiseTexture() const
	{
		return myRandomNoiseTexture;
	}

	const SC_Matrix44 SG_Shadows::GetShadowProjection(const SG_Camera& aCascadeCamera, uint aCascadeIndex) const
	{
		const SC_Matrix44 uvOffsetMatrix(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
			0.5f, 0.5f, (float)aCascadeIndex, 0.0f);

		SC_Matrix44 viewProj = (aCascadeCamera.GetView() * aCascadeCamera.GetProjection());
		viewProj = (viewProj * uvOffsetMatrix);
		return viewProj;
	}

	const SC_Matrix44 SG_Shadows::GetShadowProjection(uint aCascadeIndex) const
	{
		return GetShadowProjection(myCSMCameras[aCascadeIndex], aCascadeIndex);
	}

	const SC_Vector3f SG_Shadows::GetShadowPosition(uint aCascadeIndex) const
	{
		return myCSMCameras[aCascadeIndex].GetPosition();
	}

	const SG_Camera& SG_Shadows::GetCSMCamera(uint aCascadeIndex) const
	{
		return myCSMCameras[aCascadeIndex];
	}

	float SG_Shadows::GetCascadeSplitPoints(uint aCascadeIndex)
	{
		return myCascadeSplitPoints[aCascadeIndex + 1];
	}

	float SG_Shadows::GetPCFFilterOffset() const
	{
		float pcfFilterScale = (ourPCFFilterScaleOverride != -1.0f) ? ourPCFFilterScaleOverride : myPCFFilterScale;
		return pcfFilterScale / ourCSMResolution;
	}


	void SG_Shadows::RenderCascade(uint aIndex, SG_View* aView)
	{
		const SG_RenderData& aRenderData = aView->GetRenderData();
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();
		SR_RenderTarget* rt = myCSMArray.myRenderTargets[aIndex];
		SG_CommonRenderStates* crs = SG_CommonRenderStates::GetInstance();
		ctx->SetRenderTargets(nullptr, 0, rt, 0);
		ctx->ClearDepthTarget(rt, 0.0f, 0, 0);

		const SG_Camera& camera = aRenderData.myShadowLightCameras[aIndex];
		const SC_Matrix44& view = camera.GetView();
		const SC_Matrix44& projection = camera.GetProjection();
		const SC_Matrix44& inverseView = camera.GetInverseView();
		const SC_Matrix44& inverseProjection = camera.GetInverseProjection();

		SR_ViewConstants constants;
		constants.myWorldToClip = view * projection;
		constants.myWorldToCamera = view;
		constants.myClipToWorld = inverseProjection * inverseView;
		constants.myClipToCamera = inverseProjection;
		constants.myCameraToClip = projection;
		constants.myCameraToWorld = inverseView;
		constants.myCameraPosition = camera.GetPosition();
		constants.myCameraFov = camera.GetFovInRadians();
		constants.myCameraNear = camera.GetNear();
		constants.myCameraFar = camera.GetFar();
		constants.myResolution = SC_Vector2f(ourCSMResolution, ourCSMResolution);
		constants.myFrameIndex = (uint)aRenderData.myFrameIndex;
		ctx->BindConstantBufferRef(&constants, sizeof(constants), SR_ConstantBufferRef::SR_ConstantBufferRef_ViewConstants);

		if (aView->myWorld->myTerrain)
		{
			ctx->SetRasterizerState(crs->myCSMRasterizerState);
			aView->myWorld->myTerrain->Render(aRenderData, true);
		}

		aRenderData.myCSMQueues[aIndex].myDepth.Render("Render Depth");
	}
	void SG_Shadows::CalculateCSMViewProjections(const SG_RenderData& aRenderData)
	{
		static constexpr uint NumFrustumCorners = 8;

		if (lockCamera)
		{
			if (!wasLocked)
			{
				wasLocked = true;
				myLockedViewCamera = aRenderData.myMainCamera;
			}
		}
		else
			wasLocked = false;

		const SG_Camera& viewCamera = (lockCamera) ? myLockedViewCamera : aRenderData.myMainCamera;

		CalculateCascadeSplitPoints(viewCamera.GetNear(), SC_Min((float)ourMaxShadowRange, viewCamera.GetFar()));

		SC_Matrix44 cameraInverseView = viewCamera.GetInverseView();

		const float csmAngleThreshold = SC_Cos(SC_DegreesToRadians(ourCSMFrustumThresholdAngle));
		const SC_Vector3f lightDir = aRenderData.mySun.GetDirection().GetNormalized();

		bool forceUpdate = false;
		if (lightDir.Dot(myCachedLightDirection) < csmAngleThreshold)
		{
			myCachedLightDirection = lightDir;
			forceUpdate = true;
		}

		const int lastCascadeIndex = ourNumShadowCascades - 1;
		for (int i = 0; i < ourNumShadowCascades; ++i)
		{
			if (myCachedParams.myNeedsUpdate ||
				myCachedParams.myCachedFov != viewCamera.GetAspectRatio() ||
				myCachedParams.myCachedFov != viewCamera.GetFov())
			{
				const float cascadeNearZ = myCascadeSplitPoints[i] - myCascadeSplitPoints[i + 1] * locFade;
				const float cascadeFarZ = myCascadeSplitPoints[i + 1];
				float mbsRadius = 0.0f;
				SC_Vector3f mbsCenter(0.0f);
				if (CalculateMinBoundingSphere(viewCamera, mbsRadius, mbsCenter, cascadeNearZ, cascadeFarZ))
				{
					SC_Vector3f cameraToMBSCenter = mbsCenter - viewCamera.GetPosition();
					myCachedParams.myCameraToMBSCenter[i] = {
						cameraToMBSCenter.Dot(viewCamera.GetXAxis()),
						cameraToMBSCenter.Dot(viewCamera.GetYAxis()),
						cameraToMBSCenter.Dot(viewCamera.GetZAxis()) };

					myCachedParams.myRadius[i] = SC_Max(1.f, mbsRadius);
				}
				else
				{
					// HANDLE ERROR
				}

				myCachedParams.myCachedFov = viewCamera.GetFov();
				myCachedParams.myCachedAspectRatio = viewCamera.GetAspectRatio();
				myCachedParams.myNeedsUpdate = (i != 0);
			}

			const SC_Vector3f center(viewCamera.GetPosition() +
				viewCamera.GetXAxis() * myCachedParams.myCameraToMBSCenter[i].x +
				viewCamera.GetYAxis() * myCachedParams.myCameraToMBSCenter[i].y +
				viewCamera.GetZAxis() * myCachedParams.myCameraToMBSCenter[i].z);

			const float radius = myCachedParams.myRadius[i];
			float size = radius * 2.4f;
			const float metersPerSMTexel = size / ourCSMResolution;

			const float lastRadius = myCachedParams.myRadius[lastCascadeIndex];
			const float lastSplitPoint = myCascadeSplitPoints[lastCascadeIndex] - myCascadeSplitPoints[i + 1] * locFade;
			const float pushBackDistance = lastRadius + lastSplitPoint;

			SC_Vector3f up = (lightDir.Dot(up) < -0.999f) ? SC_Vector3f(1.0f, 0.0f, 0.0f) : SC_Vector3f(0.0f, 1.0f, 0.0f);

			bool updated = true;

			SG_Camera& shadowViewCamera = myCSMCameras[i];
			if (!forceUpdate && lightDir.Dot(shadowViewCamera.GetZAxis()) > csmAngleThreshold)
			{
				float zpos = lightDir.Dot(center);
				SC_Vector3f pos(center.Dot(shadowViewCamera.GetXAxis()), center.Dot(shadowViewCamera.GetYAxis()), zpos);

				SC_Vector3f& oldPosition = myCachedParams.myOldPositions[i];
				float xerror = (pos.x - oldPosition.x) / metersPerSMTexel;
				float yerror = (pos.y - oldPosition.y) / metersPerSMTexel;
				float zerror = (pos.z - oldPosition.z) / (radius * 0.05f);
				bool posChange = SC_Max(fabs(xerror), fabs(yerror), fabs(zerror)) > 1.0f;

				if (posChange)
				{
					pos.x = oldPosition.x + floorf(xerror + 0.5f) * metersPerSMTexel;
					pos.y = oldPosition.y + floorf(yerror + 0.5f) * metersPerSMTexel;
					pos.z = oldPosition.z + floorf(zerror + 0.5f) * radius * 0.05f;
					oldPosition = pos;

					const SC_Vector3f lightCameraPosition = shadowViewCamera.GetXAxis() * pos.x + shadowViewCamera.GetYAxis() * pos.y + shadowViewCamera.GetZAxis() * pos.z;
					const SC_Vector3f lightCameraTarget = lightCameraPosition + lightDir;

					shadowViewCamera.LookAt(lightCameraPosition, lightCameraTarget, up);
				}
				else
					updated = false;
			}
			else
			{

				SC_Vector3f camPos = viewCamera.GetPosition() + viewCamera.GetZAxis();
				SC_Vector3f oldLightCamPos = shadowViewCamera.GetPosition();
				if ((camPos - myOldValues.myOldCenters[i]).Length2() > 9)
				{
					myOldValues.myOldCenters[i] = camPos;
					myOldValues.myOldLightDirections[i] = lightDir;

					float oldXOffset = (shadowViewCamera.GetPosition() - camPos).Dot(shadowViewCamera.GetXAxis()) / metersPerSMTexel;
					oldXOffset = oldXOffset - floorf(oldXOffset);
					float oldYOffset = (shadowViewCamera.GetPosition() - camPos).Dot(shadowViewCamera.GetYAxis()) / metersPerSMTexel;
					oldYOffset = oldYOffset - floorf(oldYOffset);

					myOldValues.myOldOffsets[i] = { oldXOffset, oldYOffset };
				}

				SC_Vector3f lightCameraPosition = center;
				SC_Vector3f lightCameraTarget = lightCameraPosition + lightDir * 10;
				shadowViewCamera.LookAt(lightCameraPosition, lightCameraTarget, up);

				if (locUseCenter)
				{
					SC_Vector3f oldCenter = myOldValues.myOldCenters[i] + (lightDir - myOldValues.myOldLightDirections[i]);
					SC_Vector3f oldOffset = myOldValues.myOldOffsets[i];
					float newXOffset = (lightCameraPosition - oldCenter).Dot(shadowViewCamera.GetXAxis()) / metersPerSMTexel;
					newXOffset = newXOffset - floorf(newXOffset);
					float newYOffset = (lightCameraPosition - oldCenter).Dot(shadowViewCamera.GetYAxis()) / metersPerSMTexel;
					newYOffset = newYOffset - floorf(newYOffset);
					lightCameraPosition += shadowViewCamera.GetXAxis() * (oldOffset.x - newXOffset) * metersPerSMTexel;
					lightCameraPosition += shadowViewCamera.GetYAxis() * (oldOffset.y - newYOffset) * metersPerSMTexel;
				}

				lightCameraTarget = lightCameraPosition + lightDir * 10;
				shadowViewCamera.LookAt(lightCameraPosition, lightCameraTarget, up);
			}

			if (updated)
				shadowViewCamera.InitAsOrthogonal({ size, size }, -pushBackDistance, radius + myShadowCastDistance);

			if (locDebugDrawCameraFrustum || locDebugDrawCascadeFrustums)
			{
				SC_Vector3f shadowFrustumCorners[8];
				if (locDebugDrawCameraFrustum)
				{
					const float shadowNearZ = myCascadeSplitPoints[i] - myCascadeSplitPoints[i + 1] * locFade;
					const float shadowFarZ = myCascadeSplitPoints[i + 1];
					viewCamera.GenerateVerticesOnPlane(shadowNearZ, shadowFrustumCorners);
					viewCamera.GenerateVerticesOnPlane(shadowFarZ, shadowFrustumCorners + 4);
					locDebugDrawFrustum(shadowFrustumCorners, locCascadeColors[i]);
				}

				if (locDebugDrawCascadeFrustums)
				{
					shadowViewCamera.GenerateVerticesOnPlane(shadowViewCamera.GetNear(), shadowFrustumCorners);
					shadowViewCamera.GenerateVerticesOnPlane(shadowViewCamera.GetFar(), shadowFrustumCorners + 4);
					locDebugDrawFrustum(shadowFrustumCorners, locCascadeColors[i]);
				}
			}
		}
	}

	void SG_Shadows::CalculateCascadeSplitPoints(float aViewNearPlane, float aFarPlane)
	{
		const float splitIncrease = 1.0f / float(ourNumShadowCascades);
		float splitPoint = 0.0f;
		for (int i = 0; i <= ourNumShadowCascades; ++i)
		{
			const float logarithmicSplit = powf(aFarPlane - aViewNearPlane + 1.0f, splitPoint) + aViewNearPlane - 1.0f;
			const float linearSplit = aViewNearPlane + (aFarPlane - aViewNearPlane) * splitPoint;
			myCascadeSplitPoints[i] = logarithmicSplit * ourLinearLogSplitBalance + linearSplit * (1.0f - ourLinearLogSplitBalance);
			splitPoint += splitIncrease;
		}

	}

	bool SG_Shadows::CalculateMinBoundingSphere(const SG_Camera& aCamera, float& aRadiusOut, SC_Vector3f& aCenterOut, float aNear, float aFar)
	{
		SC_Vector3f frustumCorners[8];
		aCamera.GenerateVerticesOnPlane(aNear, frustumCorners);
		aCamera.GenerateVerticesOnPlane(aFar, frustumCorners + 4);

		const SC_Vector3f& p1 = frustumCorners[0];
		const SC_Vector3f& p2 = frustumCorners[3];
		const SC_Vector3f& p3 = frustumCorners[4];
		const SC_Vector3f& p4 = frustumCorners[7];

		// Find x and y axis for plane
		SC_Vector3f xAxis = p2 - p1;
		xAxis.Normalize();

		SC_Vector3f anotherVector = p2 - p3;
		anotherVector.Normalize();
		SC_Vector3f normal = (xAxis.Cross(anotherVector));
		normal.Normalize();
		SC_Vector3f yAxis = (xAxis.Cross(normal));
		yAxis.Normalize();

		assert(fabsf(xAxis.Dot(yAxis)) < 0.0001f);

		// Make p1 origo, and calculate plane local coordinates of the points
		SC_Vector2f localCoordinates[4];
		localCoordinates[0] = { 0.0f, 0.0f };
		localCoordinates[1] = { (p2 - p1).Length(), 0.0f };
		const SC_Vector3f origoToP3 = p3 - p1;
		localCoordinates[2] = { origoToP3.Dot(xAxis), origoToP3.Dot(yAxis) };
		const SC_Vector3f origoToP4 = p4 - p1;
		localCoordinates[3] = { origoToP4.Dot(xAxis), origoToP4.Dot(yAxis) };

		SC_GrowingArray<Circle> enclosingCircles;
		locGetMinBoundingSphereFrom2Points(localCoordinates, enclosingCircles);
		locGetMinBoundingSphereFrom3Points(localCoordinates, enclosingCircles);

		std::sort(enclosingCircles.begin(), enclosingCircles.end(), [](Circle& a, Circle& b) { return a < b; });

		if (enclosingCircles.Count() > 0)
		{
			aCenterOut = p1 + xAxis * enclosingCircles[0].myCenter.x + yAxis * enclosingCircles[0].myCenter.y;
			aRadiusOut = enclosingCircles[0].myRadius;
			return true;
		}

		return false;
	}

	bool SG_Shadows::CSMArray::Init(uint aNumCascades, uint aResolution)
	{
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();

		SR_TextureBufferDesc texBufDesc;
		texBufDesc.myFormat = SR_Format_D32_Float;
		texBufDesc.myWidth = (float)aResolution;
		texBufDesc.myHeight = (float)aResolution;
		texBufDesc.myArraySize = (uint16)aNumCascades;
		texBufDesc.myDimension = SR_Dimension_Texture2D;
		texBufDesc.myFlags = SR_ResourceFlag_AllowDepthStencil;
		texBufDesc.myMips = 1;

		myTextureBuffer = device->CreateTextureBuffer(texBufDesc, "ShadowMap Cascades");

		SR_TextureDesc textureDesc;
		textureDesc.myFormat = SR_Format_R32_Float;
		myTexture = device->CreateTexture(textureDesc, myTextureBuffer);

		if (!myTexture)
		{
			SC_ERROR_LOG("Failed to create CSM Texture.");
			return false;
		}

		myRenderTargets.RemoveAll();
		myRenderTargets.PreAllocate(aNumCascades);
		
		SR_RenderTargetDesc rtDesc;
		SR_TextureDesc mipDesc;
		mipDesc.myFormat = textureDesc.myFormat;
		mipDesc.myArraySize = textureDesc.myArraySize;
		mipDesc.myMipLevels = 1;
		mipDesc.myMostDetailedMip = 0;

		for (uint i = 0; i < aNumCascades; ++i)
		{
			rtDesc.myArrayIndex = i;
			rtDesc.myFormat = SR_Format_D32_Float;
			myRenderTargets.Add(device->CreateDepthStencil(rtDesc, myTextureBuffer)); 
			if (!myRenderTargets.GetLast())
			{
				SC_ERROR_LOG("Failed to create CSM RenderTarget.");
				return false;
			}

			mipDesc.myFirstArrayIndex = i;
			myTextureSlices.Add(device->CreateTexture(mipDesc, myTextureBuffer));
		}
		return true;
	}

	SG_Shadows::CachedCascadeParams::CachedCascadeParams()
	{
		memset(this, 0, sizeof(*this));

		myNeedsUpdate = true;
	}
}