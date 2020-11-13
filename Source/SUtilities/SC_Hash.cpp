#include "SC_Hash.h"
namespace Shift
{
	using uint16 = unsigned short;

#define GET16BITS(d) (*((const uint16 *) (d)))

	// fast hash function, from 
	// http://www.azillionmonkeys.com/qed/hash.html
	uint SC_HashData(const void* aData, uint aSize, uint aSeed)
	{
		const char* data = (const char*)aData;
		int length = aSize;
		uint hash = aSeed;

		if (length <= 0 || data == 0)
			return 0;

		int remaining = length & 3;

		length >>= 2;

		// Main loop
		for (; length > 0; length--)
		{
			hash += GET16BITS(data);
			const uint tmp = (GET16BITS(data + 2) << 11) ^ hash;
			hash = (hash << 16) ^ tmp;
			data += 2 * sizeof(uint16);
			hash += hash >> 11;
		}

		// Handle end cases
		switch (remaining)
		{
		case 3:
			hash += GET16BITS(data);
			hash ^= hash << 16;
			hash ^= data[sizeof(uint16)] << 18;
			hash += hash >> 11;
			break;
		case 2:
			hash += GET16BITS(data);
			hash ^= hash << 11;
			hash += hash >> 17;
			break;
		case 1:
			hash += *data;
			hash ^= hash << 10;
			hash += hash >> 1;
		}

		// Force "avalanching" of final 127 bits
		hash ^= hash << 3;
		hash += hash >> 5;
		hash ^= hash << 4;
		hash += hash >> 17;
		hash ^= hash << 25;
		hash += hash >> 6;

		return hash;
	}
}