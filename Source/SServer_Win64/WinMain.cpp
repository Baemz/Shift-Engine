#include "SServer_Win64_Precompiled.h"
#include "SC_Engine.h"
#include <iostream>


#if IS_WINDOWS
int APIENTRY main(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/,	LPTSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w+", stdout);

	Shift::SC_CommandLineManager::Init();

	//Shift::SEngineStartParams startParams;
	//Shift::CEngine engine;
	//
	//try
	//{
	//	Shift::StartEngine(engine, startParams);
	//}
	//catch (...)
	//{
	//	// Catch exceptions
	//}

	std::cout << "Pausing for implementation" << std::endl;
	system("pause");

	FreeConsole();

	return 0;
}
#else
int main()
{
	return 0;
}
#endif