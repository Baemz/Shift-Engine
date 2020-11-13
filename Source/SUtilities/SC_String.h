#pragma once

namespace Shift
{
	class SC_String
	{
	public:
		SC_String();
		SC_String(const char* aString);
		SC_String(const SC_String& aString);
		SC_String(SC_String&& aString) noexcept;
		~SC_String();

		const char* GetBuffer() const;
		unsigned int GetByteSize() const;
		unsigned int Count() const;
		unsigned int Length() const;

		const char& operator[](const unsigned int aIndex) const;
		char& operator[](const unsigned int aIndex);
		SC_String operator+(const SC_String& aOther);
		void operator+=(const SC_String& aOther);
		SC_String& operator=(SC_String aOther);
		bool operator==(const SC_String& aOther);
	private:
		unsigned int myCount;
		unsigned int myHashedString;
		char* myData;
	};

	SC_String operator+(const SC_String& aFirst, const SC_String& aSecond);
}