#pragma once
#include "SC_RefCounted.h"

namespace Shift
{
	template <typename T>
	class SC_Ref
	{
		template<typename U>
		struct SC_RefCountedBase
		{
			typedef SC_RefCounted Type;
		};

		template<typename U>
		struct SC_RefCountedBase<const U>
		{
			typedef const SC_RefCounted Type;
		};

		typedef typename SC_RefCountedBase<T>::Type BaseType;

		template <typename U> friend class SC_Ref;

	public:
		SC_Ref();
		SC_Ref(T* aPtr);
		SC_Ref(const SC_Ref& aPtr); 
		
		template<typename Y>
		SC_Ref(SC_Ref<Y>&& aPtr)
		{
			static_assert(std::is_convertible<Y*, T*>::value, "Pointer-types are not related.");
			myTargetAddress = aPtr.myTargetAddress;
			aPtr.myTargetAddress = nullptr;
		}

		template <typename Y>
		SC_Ref(const SC_Ref<Y>& aPtr)
			: myTargetAddress(aPtr.myTargetAddress)
		{
			AddRef();
		}

		~SC_Ref();

		void Reset(T* aPtr = nullptr);

		// Get Raw Pointer
		T* Get();
		const T* Get() const;

		// Pointer operator
		T* operator->();
		const T* operator->() const;

		// De-reference
		T& operator*();
		const T& operator*() const;

		// Assignment
		SC_Ref& operator=(T* aPtr);
		SC_Ref& operator=(const SC_Ref& aPtr);

		template <typename Y>
		SC_Ref& operator=(const SC_Ref<Y>& aPtr)
		{
			if (myTargetAddress != aPtr.myTargetAddress)
			{
				RemoveRef();
				myTargetAddress = aPtr.myTargetAddress;
				AddRef();
			}
			return *this;
		}
		
		SC_Ref& operator=(SC_Ref&& aPtr) noexcept
		{
			if (myTargetAddress != aPtr.myTargetAddress)
			{
				RemoveRef();
				myTargetAddress = aPtr.myTargetAddress;
				aPtr.myTargetAddress = nullptr;
			}
			return *this;
		}

		// Construct Raw Type
		operator T*() const 
		{ 
			return static_cast<T*>(myTargetAddress);
		}

		// Comparison
		explicit operator bool() const
		{
			return (myTargetAddress != nullptr);
		}

		template <typename Y>
		bool operator==(const SC_Ref<Y>& aPtr) const
		{
			return myTargetAddress == aPtr.myTargetAddress;
		}

		template <typename Y>
		bool operator!=(const SC_Ref<Y>& aPtr) const
		{
			return myTargetAddress != aPtr.myTargetAddress;
		}

		void AddRef();
		void RemoveRef();

	private:
		BaseType* myTargetAddress;

		struct SC_AllowRawHashing_INTERNAL;
	};
	template<class T>
	inline SC_Ref<T>::SC_Ref()
		: myTargetAddress(nullptr)
	{
	}
	template<class T>
	inline SC_Ref<T>::SC_Ref(T* aPtr)
		: myTargetAddress(aPtr)
	{
		AddRef();
	}
	template<class T>
	inline SC_Ref<T>::SC_Ref(const SC_Ref& aPtr)
		: myTargetAddress(aPtr.myTargetAddress)
	{
		AddRef();
	}
	template<class T>
	inline SC_Ref<T>::~SC_Ref()
	{
		RemoveRef();
	}
	template<class T>
	inline void SC_Ref<T>::AddRef()
	{
		if (myTargetAddress != nullptr)
			myTargetAddress->AddRef();
	}
	template<class T>
	inline void SC_Ref<T>::RemoveRef()
	{
		if (myTargetAddress != nullptr)
		{
			myTargetAddress->RemoveRef();
		}
	}
	template<class T>
	inline void SC_Ref<T>::Reset(T* aPtr)
	{
		if (aPtr != myTargetAddress)
		{
			RemoveRef();
			myTargetAddress = aPtr;
			AddRef();
		}
	}
	template<class T>
	inline T* SC_Ref<T>::Get()
	{
		return static_cast<T*>(myTargetAddress);
	}
	template<typename T>
	inline const T* SC_Ref<T>::Get() const
	{
		return static_cast<T*>(myTargetAddress);
	}
	template<class T>
	inline T* SC_Ref<T>::operator->()
	{
		return static_cast<T*>(myTargetAddress);
	}
	template<typename T>
	inline const T* SC_Ref<T>::operator->() const
	{
		return static_cast<T*>(myTargetAddress);
	}
	template<class T>
	inline T& SC_Ref<T>::operator*()
	{
		return *(static_cast<T*>(myTargetAddress));
	}
	template<typename T>
	inline const T& SC_Ref<T>::operator*() const
	{
		return *(static_cast<T*>(myTargetAddress));
	}
	template<class T>
	inline SC_Ref<T>& SC_Ref<T>::operator=(T* aPtr)
	{
		Reset(aPtr);
		return *this;
	}
	template<class T>
	inline SC_Ref<T>& SC_Ref<T>::operator=(const SC_Ref<T>& aPtr)
	{
		if (myTargetAddress != aPtr.myTargetAddress)
		{
			RemoveRef();
			myTargetAddress = aPtr.myTargetAddress;
			AddRef();
		}
		return *this;
	}
}
