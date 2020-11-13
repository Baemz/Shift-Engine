#include "SC_String.h"
#include "SC_Hash.h"
#include <cstring>
#include <algorithm>
#include <cassert>

#define STRING_BOUNDS_CHECK(aIndex)	assert(aIndex >= 0);\
									assert(aIndex < myCount)

namespace Shift
{
	char* locMergeString(unsigned int aFirstCount, void* aFirstData, unsigned int aSecondCount, void* aSecondData)
	{
		char* str = new char[(unsigned long long)aFirstCount + (unsigned long long)aSecondCount + 1];
		str[aFirstCount + aSecondCount] = '\0';
		memcpy(str, aFirstData, sizeof(char) * aFirstCount);
		memcpy(str + aFirstCount, aSecondData, sizeof(char) * aSecondCount);

		return str;
	}

	SC_String::SC_String()
		: myCount(0)
		, myData(nullptr)
		, myHashedString(0)
	{
	}

	SC_String::SC_String(const char* aString)
	{
		myCount = (unsigned int)strlen(aString);
		myData = new char[(unsigned long long)myCount + 1];
		myData[myCount] = '\0';
		memcpy(myData, aString, sizeof(char) * myCount);
		myHashedString = SC_Hash(myData);
	}
	SC_String::SC_String(const SC_String& aString)
	{
		myCount = aString.myCount;
		myData = new char[(unsigned long long)myCount + 1];
		myData[myCount] = '\0';
		memcpy(myData, aString.myData, sizeof(char) * myCount);
		myHashedString = SC_Hash(myData);
	}
	SC_String::SC_String(SC_String&& aString) noexcept
		: myCount(aString.myCount)
		, myData(aString.myData)
		, myHashedString(aString.myHashedString)
	{
		aString.myCount = 0;
		aString.myData = nullptr;
		myHashedString = 0;
	}
	SC_String::~SC_String()
	{
		myCount = 0;
		delete[] myData;
	}
	const char* SC_String::GetBuffer() const
	{
		return myData;
	}
	unsigned int SC_String::GetByteSize() const
	{
		return sizeof(char) * ((unsigned long long)myCount + 1);
	}
	unsigned int SC_String::Count() const
	{
		return myCount;
	}
	unsigned int SC_String::Length() const
	{
		return myCount;
	}
	const char& SC_String::operator[](const unsigned int aIndex) const
	{
		STRING_BOUNDS_CHECK(aIndex);
		return myData[aIndex];
	}
	char& SC_String::operator[](const unsigned int aIndex)
	{
		STRING_BOUNDS_CHECK(aIndex);
		return myData[aIndex];
	}
	SC_String SC_String::operator+(const SC_String& aOther)
	{
		char* str = locMergeString(myCount, myData, aOther.myCount, aOther.myData);

		SC_String out;
		out.myData = str;
		out.myCount = myCount + aOther.myCount;
		return out;
	}
	void SC_String::operator+=(const SC_String& aOther)
	{
		char* str = locMergeString(myCount, myData, aOther.myCount, aOther.myData);

		delete[] myData;
		myData = str;
		myCount += aOther.myCount;
		myHashedString = SC_Hash(myData);
	}
	SC_String& SC_String::operator=(SC_String aOther)
	{
		std::swap(myData, aOther.myData);
		std::swap(myCount, aOther.myCount);
		std::swap(myHashedString, aOther.myHashedString);
		return *this;
	}
	bool SC_String::operator==(const SC_String& aOther)
	{
		if (myHashedString == aOther.myHashedString)
			return true;

		return false;
	}
	SC_String operator+(const SC_String& /*aFirst*/, const SC_String& /*aSecond*/)
	{
		return SC_String();
	}
}