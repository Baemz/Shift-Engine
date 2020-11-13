#pragma once
#include "SC_Vector.h"

namespace Shift
{
	template<class Type>
	class SC_VolumeObject
	{
	public:
		SC_VolumeObject() {}
		~SC_VolumeObject() {}

		Type myObject;
		SC_Vector2f myPoint;
		SC_Vector2f mySize;
	};
}