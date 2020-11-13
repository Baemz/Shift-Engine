#pragma once
namespace Shift
{
	enum SC_FileState
	{
		SC_FileState_OpenRead,
		SC_FileState_OpenWrite,
		SC_FileState_Closed,

		SC_FileState_Count,
		SC_FileState_Unknown = SC_FileState_Count,
	};

	class SC_File
	{
	public:
		SC_File();
		~SC_File();

		bool Open(const char* aFilepath);
		void Close();

		void Seek();

		void Read();
		void Write();
	private:
		SC_FileState myState;
	};
}