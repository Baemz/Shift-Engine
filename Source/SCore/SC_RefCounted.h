#pragma once
namespace Shift
{
	class SC_RefCounted
	{
	public:
		SC_RefCounted();

		SC_RefCounted(const SC_RefCounted&)
			: myRefCount(InitValue)
		{
		}

		SC_RefCounted& operator=(const SC_RefCounted&) { return *this; }

		void AddRef();
		void RemoveRef();


		uint GetRefCount() const;

	protected:
		virtual ~SC_RefCounted();
		virtual bool CanRelease(volatile uint& aRefCount);

		volatile uint myRefCount;
		static constexpr uint InitValue = SC_UINT_MAX;
	};
}