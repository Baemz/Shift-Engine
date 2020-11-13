#pragma once

namespace Shift
{
	template<typename T>
	class SC_UniquePtr
	{
	public:
		SC_UniquePtr()
			: myTarget(nullptr)
		{}

		~SC_UniquePtr()
		{
			delete myTarget;
		}

		SC_UniquePtr(T* aPtr)
			: myTarget(aPtr)
		{

		}

		SC_UniquePtr(const SC_UniquePtr&) = delete;
		void operator=(const SC_UniquePtr&) = delete;

		template <typename Y>
		inline SC_UniquePtr& operator=(Y* aPtr)
		{
			Reset(static_cast<T*>(aPtr));
			return *this;
		}


		void Reset(T* aPtr = nullptr);

		T* Get();
		const T* Get() const;

		T* operator->();
		const T* operator->() const;

		T& operator*();
		const T& operator*() const; 
		
		operator T* () const
		{
			return myTarget;
		}

		explicit operator bool() const
		{
			return (myTarget != nullptr);
		}

		// WARNING! Transfers ownership to target.
		void Copy(const SC_UniquePtr& aPtr);

	private:
		T* myTarget;
	};
	template<typename T>
	inline void SC_UniquePtr<T>::Reset(T* aPtr)
	{
		if (myTarget != aPtr)
		{
			delete myTarget;
			myTarget = aPtr;
		}
	}
	template<typename T>
	inline T* SC_UniquePtr<T>::Get()
	{
		return myTarget;
	}
	template<typename T>
	inline const T* SC_UniquePtr<T>::Get() const
	{
		return myTarget;
	}
	template<typename T>
	inline T* SC_UniquePtr<T>::operator->()
	{
		return myTarget;
	}
	template<typename T>
	inline const T* SC_UniquePtr<T>::operator->() const
	{
		return myTarget;
	}
	template<typename T>
	inline T& SC_UniquePtr<T>::operator*()
	{
		return *myTarget;
	}
	template<typename T>
	inline const T& SC_UniquePtr<T>::operator*() const
	{
		return *myTarget;
	}
	template<typename T>
	inline void SC_UniquePtr<T>::Copy(const SC_UniquePtr& aPtr)
	{
		myTarget = aPtr.myTarget;
		aPtr.myTarget = nullptr;
	}
}