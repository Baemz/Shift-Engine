#pragma once

namespace physx
{
	class PxMaterial;
}

namespace Shift
{
	struct SP_MaterialDesc
	{
		SP_MaterialDesc()
			: myStaticFriction(0.0f)
			, myDynamicFriction(0.0f)
			, myRestitution(0.0f)
		{}
		float myStaticFriction;
		float myDynamicFriction;
		float myRestitution;
	};


	class SP_Material : public SC_RefCounted
	{
		friend class SP_PhysicsDevice;
	public:
		SP_Material();
		~SP_Material();

	private:
		physx::PxMaterial* myPxMaterial;
	};
}
