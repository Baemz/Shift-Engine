#pragma once

namespace Shift
{
	class SC_Bitset
	{
	public:
		SC_Bitset();
		SC_Bitset(unsigned int aNumBits);
		SC_Bitset(const SC_Bitset& aOther);

		~SC_Bitset();

		const bool operator[](unsigned int aIndex) const;

	private:
	};
}