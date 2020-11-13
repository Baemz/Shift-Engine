#include "SC_Ray.h"

namespace Shift
{
	SC_Ray::SC_Ray()
	{
	}

	SC_Ray::SC_Ray(const SC_Vector3f& aOrigin, const SC_Vector3f& aDirection)
		: myOrigin(aOrigin)
		, myDirection(aDirection)
	{
	}

	SC_Ray::~SC_Ray()
	{
	}
}