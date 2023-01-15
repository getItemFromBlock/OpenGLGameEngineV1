#include "Core/App/App.hpp"

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#include "Core/Debug/Log.hpp"
#include "Core/Debug/Assert.hpp"
#include "Core/Maths/Maths.hpp"

#ifdef GAME
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

int main(int argc, char** argv)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(163);
#endif
	{
		Core::App::AppInit data = { 1200, 900, 4, 5, "Super Galaxy 64", Core::Debug::Log::glDebugOutput };
		Core::App::App app = Core::App::App();
		Core::Debug::Log::OpenFile("Logs/output");

		int out = app.InitApp(data);
		if (out) return out;

		Core::Debug::Log::Print("OPENGL Version: %s\n", glGetString(GL_VERSION));

		app.Update();

		app.ClearApp();
		Core::Debug::Log::CloseFile();
	}
	return 0;
}