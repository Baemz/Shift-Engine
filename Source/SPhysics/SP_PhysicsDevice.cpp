#include "SPhysics_Precompiled.h"
#include "SP_PhysicsDevice.h"

#include "SP_RigidBody.h"
#include "SP_Material.h"

#define CHECK_PTR_VALIDATION(ptr) assert("Pointer Invalid" && (ptr != nullptr))

namespace Shift
{
	static physx::PxDefaultErrorCallback  DefaultErrorCallback;
	static physx::PxDefaultAllocator  DefaultAllocatorCallback;

	SP_PhysicsDevice* SP_PhysicsDevice::ourInstance = nullptr;

	SP_PhysicsDevice::SP_PhysicsDevice()
		: myFoundation(nullptr)
		, myPhysics(nullptr)
		, myScene(nullptr)
		, myTimeAccumulation(0)
	{
	}


	SP_PhysicsDevice::~SP_PhysicsDevice()
	{
		SP_INTERNAL_SAFE_REALEASE(myCpuDispatcher);
		SP_INTERNAL_SAFE_REALEASE(myScene);
		SP_INTERNAL_SAFE_REALEASE(myPhysics);
		SP_INTERNAL_SAFE_REALEASE(myFoundation);
	}

	bool SP_PhysicsDevice::Init()
	{
		myFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, DefaultAllocatorCallback, DefaultErrorCallback);
		if (!myFoundation)
		{
			SC_ERROR_LOG("PxCreateFoundation failed");
			return false;
		}

		bool recordMemoryAllocations = true;

		myPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *myFoundation, physx::PxTolerancesScale(), recordMemoryAllocations, nullptr);
		if (myPhysics == nullptr)
		{
			SC_ERROR_LOG("PxCreatePhysics failed");
			return false;
		}

		physx::PxSceneDesc sceneDesc(myPhysics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -0.981f, 0.0f);
		myCpuDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		sceneDesc.cpuDispatcher = myCpuDispatcher;
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		myScene = myPhysics->createScene(sceneDesc);
		return true;
	}

	void SP_PhysicsDevice::Create()
	{
		if (!ourInstance)
			ourInstance = new SP_PhysicsDevice();
		else
			assert(false && "Already created PhysicsEngine");

		if (ourInstance && !ourInstance->Init())
			assert(false && "Failed to init CPhysicsEngine");
			
	}

	void SP_PhysicsDevice::Destroy()
	{
		delete ourInstance;
		ourInstance = nullptr;
	}

	void SP_PhysicsDevice::Update(float aDeltaTime)
	{
		SC_PROFILER_FUNCTION();
		myTimeAccumulation += aDeltaTime;
		if (myTimeAccumulation < myStepSize)
			return;

		myTimeAccumulation -= myStepSize;
		myScene->simulate(myStepSize);
		myScene->fetchResults(true);
	}

	SC_Ref<SP_RigidBody> SP_PhysicsDevice::CreateDynamicRigidBody()
	{
		SC_Ref<SP_RigidBody> rigidBody = new SP_RigidBody();

		rigidBody->myPxMaterial = myPhysics->createMaterial(0.5f, 0.5f, 0.6f);

		PxRigidDynamic* dynamic = PxCreateDynamic(*myPhysics, PxTransform(PxVec3(0, 0, 0)), PxSphereGeometry(10), *rigidBody->myPxMaterial, 10.f);
		dynamic->setAngularDamping(0.5f);
		rigidBody->myPxRigidBody = dynamic;
		rigidBody->myPxScene = myScene;
		myScene->addActor(*dynamic);

		return rigidBody;
	}
	SC_Ref<SP_RigidBody> SP_PhysicsDevice::CreateStaticRigidBody()
	{
		SC_Ref<SP_RigidBody> rigidBody = new SP_RigidBody();

		rigidBody->myPxMaterial = myPhysics->createMaterial(0.5f, 0.5f, 0.6f);
		PxRigidStatic* staticRigidBody = PxCreateStatic(*myPhysics, PxTransform(PxVec3(0, 0, 0)), PxSphereGeometry(10), *rigidBody->myPxMaterial);
		rigidBody->myPxRigidBody = staticRigidBody;
		rigidBody->myPxScene = myScene;
		myScene->addActor(*staticRigidBody);
		return rigidBody;
	}
	SC_Ref<SP_Material> SP_PhysicsDevice::CreateMaterial(const SP_MaterialDesc& aDesc)
	{
		SC_Ref<SP_Material> material = new SP_Material();
		material->myPxMaterial = myPhysics->createMaterial(aDesc.myStaticFriction, aDesc.myDynamicFriction, aDesc.myRestitution);

		return material;
	}
}