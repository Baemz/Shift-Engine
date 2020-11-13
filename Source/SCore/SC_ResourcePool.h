#pragma once
namespace Shift
{
	class SC_Resource;
	class SC_ResourcePool
	{
	public:
		~SC_ResourcePool();

		static void CreateResource(SC_Resource* aResource);
		static void RemoveResource(SC_Resource* aResource);
	private:
		SC_ResourcePool();
	};
}