#pragma once

namespace physx
{
	class PxRigidActor;
	class PxScene;
	class PxMaterial;
}

namespace Shift
{
	class SP_RigidBody : public SC_RefCounted
	{
		friend class SP_PhysicsDevice;
	public:
		SP_RigidBody();
		~SP_RigidBody();

		SC_Vector3f GetPosition() const;

	private:
		physx::PxRigidActor* myPxRigidBody;
		physx::PxMaterial* myPxMaterial;
		physx::PxScene* myPxScene;
	};
}