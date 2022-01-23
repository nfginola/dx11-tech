#include "pch.h"
#include "Application.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

int main()
{
	// https://docs.microsoft.com/en-us/visualstudio/debugger/finding-memory-leaks-using-the-crt-library?view=vs-2022
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	/*
		Note! Exiting the Console will lead to memory leaks because
		it never exits properly through here.
		Try set a debug point on CrtDumpMemoryLeaks.
		The app doesnt reach it if closed through the terminal..

		Consider using SubSystem: Windows instead of Console and open up a 
		console manually.
	*/

	// Destructor should be called before dumping memory leaks.
	{
		unique_ptr<Application> app = make_unique<Application>();
		app->run();
	}

	_CrtDumpMemoryLeaks();

	return 0;
}