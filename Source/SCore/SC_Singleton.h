#pragma once

namespace Shift
{
	template <typename Type>
	class SC_SimpleSingleton
	{
	public:
		static Type& GetInstance()
		{
			static Type instance;
			return instance;
		}
	};

	template <typename Type>
	class SC_Singleton
	{
	public:
		static Type* GetInstance() { return ourInstance; }

	protected:
		static Type* ourInstance;
	};

	template <typename Type>
	Type* SC_Singleton<Type>::ourInstance = nullptr;
}