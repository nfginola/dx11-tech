#include "pch.h"
#include "Application.h"

#include "Graphics/dx.h"

int main()
{
	unique_ptr<Application> app = make_unique<Application>();
	app->run();

	return 0;
}