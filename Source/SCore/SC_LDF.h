#pragma once

/*
	LDF (Large Distribution File)
	-------------------------------------------
	Custom file format used to store game-ready data. Data is packed into custom layouts to optimize
	storage footprint and loading speed.

	- ToC		- (Table of Content)
	- Data 0	- Data Collection part
	- Data 1	- Data Collection part
	- Data 2	- Data Collection part
	  ...
	- Data <x>	- Data Collection part
	- <eof>		- End of file
*/

namespace Shift
{
	class SC_LDF
	{
	public:
		SC_LDF();
		~SC_LDF();

	private:

	};
}

