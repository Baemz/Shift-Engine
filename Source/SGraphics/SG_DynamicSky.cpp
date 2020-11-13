#include "SGraphics_Precompiled.h"
#include "SG_DynamicSky.h"
#include "SG_View.h"
#include "SG_Camera.h"

#include "SR_GraphicsDevice.h"
#include "SR_ShaderCompiler.h"

#include <time.h>
#include "SG_EnvironmentConstants.h"
#include "SR_ViewConstants.h"

static constexpr float locSunArcminuteSize = 32.f;

namespace Shift
{
	struct SDate
	{
		uint myDay;
		uint myMonth;
		uint myYear;
	};
	SDate locGetCurrentDate()
	{
		::time_t rawtime;
		::tm* _date;

		::time(&rawtime);
		_date = ::localtime(&rawtime);

		SDate date;
		date.myYear = _date->tm_year + 1900;
		date.myMonth = _date->tm_mon + 1;
		date.myDay = _date->tm_mday;

		return date;
	}

	double locGetDay(uint aDay, uint aMonth, uint aYear, double aTime)
	{
		uint year = aYear;
		uint month = aMonth;
		uint day = aDay;

		uint d = 367 * year - 7 * (year + (month + 9) / 12) / 4 + 275 * month / 9 + day - 730530;
		return d + aTime / 24.0;
	}

	SC_Matrix33 SG_DynamicSky::CalculateNorthAlignmentRotation()
	{
		return SC_Matrix33::CreateRotationY(SC_DegreesToRadians(-myZDirection));
	}

	SC_Vector3f SG_DynamicSky::GetCelestialObjectDirection(float aLST, float aRA, float aDec)
	{
		// http://www.stjarnhimlen.se/comp/ppcomp.html

		float latitudeDeg = myLatitudeN;
		const float pi = SC_PI_FLOAT;

		// 12b. Azimuthal coordinates

		// To find the azimuthal coordinates (azimuth and altitude) we proceed by computing the HA (Hour Angle) of the object
		float HA = aLST * (pi / 12) - aRA;

		float x = cos(HA) * cos(aDec);
		float y = sin(HA) * cos(aDec);
		float z = sin(aDec);

		float lat = (pi / 180) * latitudeDeg;
		float xhor = x * sin(lat) - z * cos(lat);
		float yhor = y;
		float zhor = x * cos(lat) + z * sin(lat);

		SC_Vector3f ret(-yhor, zhor, -xhor);
		ret *= CalculateNorthAlignmentRotation();
		return ret;
	}

	SC_Vector3f SG_DynamicSky::CalculateToSunDirection(float aTime, float& aLSTOut)
	{
		// http://www.stjarnhimlen.se/comp/ppcomp.html

		float UTCOffset = myUTCOffset;
		double UT = aTime - UTCOffset;

		const double d = locGetDay(myDay, myMonth, myYear, UT);
		const double pi = SC_PI_DOUBLE;

		float longitudeDeg = myLongitudeE;

		double w = (pi / 180) * (282.9404 + 4.70935E-5 * d); // argument of perihelion
		double e = 0.016709 - 1.151E-9 * d; // eccentricity (0=circle, 0-1=ellipse, 1=parabola)
		double M = (pi / 180) * (356.0470 + 0.9856002585 * d); // mean anomaly (0 at perihelion; increases uniformly with time)

		double ecl = (pi / 180) * (23.4393 - 3.563E-7 * d); // obliquity of the ecliptic, i.e. the "tilt" of the Earth's axis of rotation

		// 5. The position of the Sun

		// First, compute the eccentric anomaly E from the mean anomaly M and from the eccentricity e:
		double E = M + e * sin(M) * (1.0 + e * cos(M));

		// Then compute the Sun's distance r and its true anomaly v from:
		double xv = cos(E) - e;
		double yv = sqrt(1.0 - e * e) * sin(E);

		double v = atan2(yv, xv);
		double r = sqrt(xv * xv + yv * yv);

		// Now, compute the Sun's true longitude:
		double lonsun = v + w;

		// Convert lonsun,r to ecliptic rectangular geocentric coordinates xs,ys:
		double xs = r * cos(lonsun);
		double ys = r * sin(lonsun);

		// (since the Sun always is in the ecliptic plane, zs is of course zero).
		// xs,ys is the Sun's position in a coordinate system in the plane of the ecliptic. To convert this to equatorial, rectangular, geocentric coordinates, compute:

		double xe = xs;
		double ye = ys * cos(ecl);
		double ze = ys * sin(ecl);

		// Finally, compute the Sun's Right Ascension (RA) and Declination (Dec):
		double RA = atan2(ye, xe);
		double Dec = atan2(ze, sqrt(xe * xe + ye * ye));

		// 5b. The Sidereal Time

		// We also need the Sun's mean longitude, Ls, which can be computed from the Sun's v and w as follows:
		double Ls = v + w;

		double GMST0 = Ls * (12 / pi) + 12; // Greenwich Mean Sidereal Time at 0h UT (GMST0) is, as the name says, the GMST at Greenwich Midnight
		double GMST = GMST0 + UT; // The Greenwich Mean Sideral Time (GMST) is the LST at Greenwich
		double LST = GMST + longitudeDeg / 15; // The Local Sideral Time (LST) is simply the RA of your local meridian
		aLSTOut = float(LST);

		return GetCelestialObjectDirection(aLSTOut, float(RA), float(Dec));
	}


	SG_DynamicSky::SG_DynamicSky()
		: mySunRadiusDivDistance(0.0001f)
		, myTimeOfDay(10.0f)
		, myTimeOfDaySpeed(0.0f)
		, myAmbientLightIntensity(1.0f)
	{
	}


	SG_DynamicSky::~SG_DynamicSky()
	{
	}
	void SG_DynamicSky::Init()
	{
		SR_GraphicsDevice* device = SR_GraphicsDevice::GetDevice();
		SR_ShaderCompiler* shaderCompiler = device->GetShaderCompiler();

		SR_ShaderStateDesc mySkyShaderDesc;
		mySkyShaderDesc.myShaderByteCodes[SR_ShaderType_Compute] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/Environment/Sky_Compute.ssf", SR_ShaderType_Compute);
		mySkyShader = device->CreateShaderState(mySkyShaderDesc);

		SR_ShaderStateDesc mySkyProbeShaderDesc;
		mySkyProbeShaderDesc.myShaderByteCodes[SR_ShaderType_Compute] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/Environment/SkyProbe_Compute.ssf", SR_ShaderType_Compute);
		mySkyProbeShader = device->CreateShaderState(mySkyProbeShaderDesc);

		SR_ShaderStateDesc mySkyProbeIrradianceShaderDesc;
		mySkyProbeIrradianceShaderDesc.myShaderByteCodes[SR_ShaderType_Compute] = shaderCompiler->CompileShaderFromFile("ShiftEngine/Shaders/SGraphics/Environment/Compute_Irradiance.ssf", SR_ShaderType_Compute);
		mySkyProbeIrradianceShader = device->CreateShaderState(mySkyProbeIrradianceShaderDesc);

		mySkyTexture = device->GetCreateTexture("skySpecularHDR.dds");

		// Set coordinates for current date in Malmö
		myLatitudeN = 55.6050f;
		myLongitudeE = 13.0038f;

		SDate date = locGetCurrentDate();
		myDay = date.myDay;
		myMonth = date.myMonth;
		myYear = date.myYear;

		if (myMonth > 3 && myMonth < 11) // "rounded" CEST date span
			myUTCOffset = 2.0f; // CEST
		else
			myUTCOffset = 1.0f; // CET

		mySunLightColorConst = SC_Vector3f(1.f, 0.97f, 0.86f);
		mySunIntensity = 1.5f;

		//#if IS_FINAL_BUILD
		//		myTimeOfDaySpeed = 0.6f;
		//#endif
	}

	void SG_DynamicSky::PrepareEnvironmentConstants(SG_EnvironmentConstants& aConstants)
	{
		aConstants.myDirectSunFactor = myDirectSunFactor;
		aConstants.mySunCosHigh = mySunCosHigh;
		aConstants.mySunCosLow = mySunCosLow;
		aConstants.myToSunDirection = myToSunDirection;
		aConstants.mySunColor = mySunLightColor;
		aConstants.mySunIntensity = mySunIntensity;
		aConstants.myAmbientLightIntensity = myAmbientLightIntensity;
	}

	void SG_DynamicSky::Update(float /*aDeltaTime*/)
	{
		// TEMP
		static const float directSunMultiplier = (4 * SC_PI_FLOAT) / (mySunRadiusDivDistance * mySunRadiusDivDistance);

		myTimeOfDay += SC_Timer::GetDeltaTime() * myTimeOfDaySpeed;
		myDirectSunFactor = mySunIntensity * directSunMultiplier;

		mySunLightColor = mySunLightColorConst * mySunIntensity;

		float LST;
		myToSunDirection = CalculateToSunDirection(myTimeOfDay, LST);

		//// Static top->down view for static sun-light
		//myToSunDirection = -(SC_Vector3f(0.12f, -1.f, -0.1f).GetNormalized());

		mySunCosLow = SC_Cos(SC_DegreesToRadians(locSunArcminuteSize * 1.2f) / 120.f);
		mySunCosHigh = SC_Cos(SC_DegreesToRadians(locSunArcminuteSize) / 120.f);
	}

	void SG_DynamicSky::Render(SG_View* aView)
	{
		const SC_Float2 fullResolution = SR_GraphicsDevice::GetDevice()->GetResolution();
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();

		ctx->BindTexture(mySkyTexture, 0);
		ctx->BindTextureRW(aView->myFrameResources.myFullscreenHDR.myTextureRW, 0);
		ctx->SetShaderState(mySkyShader);

		ctx->Dispatch((uint)ceil(fullResolution.x / 8.f), (uint)ceil(fullResolution.y / 8.f), 1);
	}

	void SG_DynamicSky::RenderSkyProbeFace(SG_Camera& aCamera, SR_Texture* aFaceRT, const uint aFaceIndex)
	{
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();

		const SC_Matrix44& view = aCamera.GetView();
		const SC_Matrix44& projection = aCamera.GetProjection();
		const SC_Matrix44& inverseView = aCamera.GetInverseView();
		const SC_Matrix44& inverseProjection = aCamera.GetInverseProjection();

		SR_ViewConstants viewConstants;
		viewConstants.myWorldToClip = view * projection;
		viewConstants.myWorldToCamera = view;
		viewConstants.myClipToWorld = inverseProjection * inverseView;
		viewConstants.myClipToCamera = inverseProjection;
		viewConstants.myCameraToClip = projection;
		viewConstants.myCameraToWorld = inverseView;
		viewConstants.myCameraPosition = aCamera.GetPosition();
		viewConstants.myCameraFov = aCamera.GetFovInRadians();
		viewConstants.myCameraNear = aCamera.GetNear();
		viewConstants.myCameraFar = aCamera.GetFar();
		viewConstants.myResolution = SR_GraphicsDevice::GetDevice()->GetResolution();
		viewConstants.myFrameIndex = 0;

		ctx->BindConstantBufferRef(&viewConstants, sizeof(viewConstants), SR_ConstantBufferRef::SR_ConstantBufferRef_ViewConstants);

		struct ProbeConstants
		{
			int faceIndex;            
		} probeConstants;
		probeConstants.faceIndex = aFaceIndex;

		ctx->BindConstantBuffer(&probeConstants, sizeof(probeConstants), 0);

		ctx->BindTexture(mySkyTexture, 0);
		ctx->BindTextureRW(aFaceRT, 0);
		ctx->SetShaderState(mySkyProbeShader);

		ctx->Dispatch((uint)ceil(256 / 8.f), (uint)ceil(256 / 8.f), 1);
	}

	void SG_DynamicSky::RenderSkyProbeIrradianceFace(SG_Camera& aCamera, SR_Texture* aFaceRT, SR_Texture* aSkyTexture, const uint aFaceIndex)
	{
		SR_GraphicsContext* ctx = SR_GraphicsContext::GetCurrent();

		const SC_Matrix44& view = aCamera.GetView();
		const SC_Matrix44& projection = aCamera.GetProjection();
		const SC_Matrix44& inverseView = aCamera.GetInverseView();
		const SC_Matrix44& inverseProjection = aCamera.GetInverseProjection();

		SR_ViewConstants viewConstants;
		viewConstants.myWorldToClip = view * projection;
		viewConstants.myWorldToCamera = view;
		viewConstants.myClipToWorld = inverseProjection * inverseView;
		viewConstants.myClipToCamera = inverseProjection;
		viewConstants.myCameraToClip = projection;
		viewConstants.myCameraToWorld = inverseView;
		viewConstants.myCameraPosition = aCamera.GetPosition();
		viewConstants.myCameraFov = aCamera.GetFovInRadians();
		viewConstants.myCameraNear = aCamera.GetNear();
		viewConstants.myCameraFar = aCamera.GetFar();
		viewConstants.myResolution = SR_GraphicsDevice::GetDevice()->GetResolution();
		viewConstants.myFrameIndex = 0;

		ctx->BindConstantBufferRef(&viewConstants, sizeof(viewConstants), SR_ConstantBufferRef::SR_ConstantBufferRef_ViewConstants);

		struct ProbeConstants
		{
			int faceIndex;
		} probeConstants;
		probeConstants.faceIndex = aFaceIndex;

		ctx->BindConstantBuffer(&probeConstants, sizeof(probeConstants), 0);

		ctx->BindTexture(aSkyTexture, 0);
		ctx->BindTextureRW(aFaceRT, 0);
		ctx->SetShaderState(mySkyProbeIrradianceShader);

		ctx->Dispatch((uint)ceil(256 / 8.f), (uint)ceil(256 / 8.f), 1);
	}

	void SG_DynamicSky::OverrideTimeOfDay()
	{
	}
	float SG_DynamicSky::GetTimeOfDay() const
	{
		return myTimeOfDay;
	}
	SC_Vector3f SG_DynamicSky::GetToSunDirection() const
	{
		return myToSunDirection;
	}
}