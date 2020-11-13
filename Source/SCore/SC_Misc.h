#pragma once

namespace Shift
{  
	// Protection from unintended ADL
	namespace Noncopyable_
	{
		class SC_Noncopyable
		{
		protected:
			SC_Noncopyable() {}
			~SC_Noncopyable() {}
		private:
			SC_Noncopyable(const SC_Noncopyable&);
			const SC_Noncopyable& operator=(const SC_Noncopyable&);
		};
	}
	typedef Noncopyable_::SC_Noncopyable SC_Noncopyable;

	int SC_FindFirstBit(uint8 aBitField);
	int SC_FindFirstBit(uint16 aBitField);
	int SC_FindFirstBit(uint32 aBitField);
	int SC_FindFirstBit(uint64 aBitField);
	int SC_FindLastBit(uint8 aBitField);
	int SC_FindLastBit(uint16 aBitField);
	int SC_FindLastBit(uint32 aBitField);
	int SC_FindLastBit(uint64 aBitField);

}