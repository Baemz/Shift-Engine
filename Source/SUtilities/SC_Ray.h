#pragma once
#include "SC_Vector.h"

namespace Shift
{
	class SC_Ray
	{
	public:
		SC_Ray();
		SC_Ray(const SC_Vector3f& aOrigin, const SC_Vector3f& aDirection);

		~SC_Ray();

		SC_Vector3f myOrigin;
		SC_Vector3f myDirection;

	private:
	};
}
