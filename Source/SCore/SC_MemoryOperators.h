#pragma once

#include "SC_Memory.h"

#ifdef SC_OVERLOAD_MEMORY_OPERATORS

#ifdef new
#undef new
#endif

void* operator new(size_t aSize)
{
	return Shift::SC_Memory::Allocate(aSize, false);
}

void* operator new[](size_t aSize)
{
	return Shift::SC_Memory::Allocate(aSize, true);
}

void* operator new(std::size_t aSize, const std::nothrow_t&) throw()
{
	return Shift::SC_Memory::Allocate(aSize, false);
}

void* operator new[](std::size_t aSize, const std::nothrow_t&) throw()
{
	return Shift::SC_Memory::Allocate(aSize, true);
}

// Debug new
void* operator new(size_t aSize, int /*aBlockUse*/, const char* /*aFileName*/, int /*aLine*/)
{
	return Shift::SC_Memory::Allocate(aSize, false);
}

// Debug new[]
void* operator new[](size_t aSize, int /*aBlockUse*/, const char* /*aFileName*/, int /*aLine*/)
{
	return Shift::SC_Memory::Allocate(aSize, true);
}

void* operator new(size_t aSize, const char* /*aFileName*/, int /*aLine*/)
{
	return Shift::SC_Memory::Allocate(aSize, false);
}

void* operator new[](size_t aSize, const char* /*aFileName*/, int /*aLine*/)
{
	return Shift::SC_Memory::Allocate(aSize, true);
}

void operator delete(void* aPointer) noexcept
{
	Shift::SC_Memory::Deallocate(aPointer, false);
}

void operator delete[](void* aPointer) noexcept
{
	Shift::SC_Memory::Deallocate(aPointer, true);
}

void operator delete(void* aPointer, size_t /*aSize*/)
{
	Shift::SC_Memory::Deallocate(aPointer, false);
}

void operator delete[](void* aPointer, size_t /*aSize*/)
{
	Shift::SC_Memory::Deallocate(aPointer, true);
}

void operator delete(void* aPointer, int /*aBlockUse*/, const char* /*aFileName*/, int /*aLine*/)
{
	Shift::SC_Memory::Deallocate(aPointer, false);
}

void operator delete[](void* aPointer, int /*aBlockUse*/, const char* /*aFileName*/, int /*aLine*/)
{
	Shift::SC_Memory::Deallocate(aPointer, true);
}

void operator delete(void* aPointer, const char* /*aFileName*/, int /*aLine*/)
{
	Shift::SC_Memory::Deallocate(aPointer, false);
}

void operator delete[](void* aPointer, const char* /*aFileName*/, int /*aLine*/)
{
	Shift::SC_Memory::Deallocate(aPointer, true);
}
#endif // SC_OVERLOAD_MEMORY_OPERATORS