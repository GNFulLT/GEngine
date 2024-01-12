#include "editor/editor_application_impl.h"
#include "engine/gengine.h"
#include "game.h"

#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <filesystem>
#include <dylib.hpp>


typedef void*(GAPPLICATION_CREATION_FUNC_TYPE)();


int main()
{
#ifdef WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN ,_CRTDBG_MODE_DEBUG);
#endif

	auto engine = create_the_engine();
	GApplicationImpl* impl = nullptr;
	bool isEditor = false;
	auto dllPath = std::filesystem::current_path() / "ExampleProject2" /"ExampleProject.gproject";

	if (std::filesystem::exists(dllPath))
	{
		impl = new GGameInpl(dllPath);
	}
	else
	{
		impl = create_the_editor();
	}


	engine->init(impl);

	engine->run();


	delete impl;
	delete engine;

	_CrtDumpMemoryLeaks();
	return 0;
}