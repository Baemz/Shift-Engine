#include "SClient_Linux_Precompiled.h"
#include <iostream>

void InitApplication()
{

}

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	InitApplication();


	//auto& allocations = Shift::SMemoryLeakDetection::__GetMemoryAllocations();
	
	
	//for (auto& allocation : allocations)
	//{
	//	S_ERROR_LOG("[MEM-LEAK] File: (%s) - Line: (%i)", allocation.second.first.c_str(), (int)allocation.second.second);
	//	SE_UNUSED(allocation);
	//}
	//if (allocations.empty() == false)
	//{
	//	system("pause");
	//}
	std::cout << "Pausing for implementation" << std::endl;
	system("pause");

	return 0;
}
