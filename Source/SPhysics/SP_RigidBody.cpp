#include "SPhysics_Precompiled.h"
#include "SP_RigidBody.h"

namespace Shift
{
	SP_RigidBody::SP_RigidBody()
		: myPxRigidBody(nullptr)
		, myPxMaterial(nullptr)
		, myPxScene(nullptr)
	{
	}

	SP_RigidBody::~SP_RigidBody()
	{
		myPxScene->removeActor(*myPxRigidBody);

		SP_INTERNAL_SAFE_REALEASE(myPxRigidBody);
		SP_INTERNAL_SAFE_REALEASE(myPxMaterial);
		SP_INTERNAL_SAFE_REALEASE(myPxScene);
	}

	SC_Vector3f SP_RigidBody::GetPosition() const
	{
		PxVec3 position = myPxRigidBody->getGlobalPose().p;
		return SC_Vector3f(position.x, position.y, position.z);
	}
}