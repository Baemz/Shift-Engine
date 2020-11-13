#pragma once

namespace physx
{
	class PxFoundation;
	class PxPhysics;
	class PxScene;
	class PxDefaultCpuDispatcher;
}

namespace Shift
{
	class SP_RigidBody;
	class SP_Material;
	struct SP_MaterialDesc;

	//struct SP_DeviceInitDesc
	//{
	//
	//};

	class SP_PhysicsDevice
	{
	public:
		SP_PhysicsDevice();
		~SP_PhysicsDevice();

		bool Init();

		void Update(float aDeltaTime);

		SC_Ref<SP_RigidBody> CreateDynamicRigidBody();
		SC_Ref<SP_RigidBody> CreateStaticRigidBody();

		SC_Ref<SP_Material> CreateMaterial(const SP_MaterialDesc& aDesc);

		static void Create();
		static void Destroy();
		static SP_PhysicsDevice* GetInstance() { return ourInstance; }
	private:

		static SP_PhysicsDevice* ourInstance;

		const float myStepSize = 1 / 60.f;
		float myTimeAccumulation;

		physx::PxFoundation* myFoundation;
		physx::PxPhysics* myPhysics;
		physx::PxScene* myScene;
		physx::PxDefaultCpuDispatcher* myCpuDispatcher;
	};

}