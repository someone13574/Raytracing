#pragma once

#include "Win.h"

#include "EngineLogger.h"
#include "Exceptions/EngineException.h"

extern Engine::Application* Engine::CreateApplication();

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	try
	{
		EngineDebug::EngineLogger::ClearLog();

		auto application = Engine::CreateApplication();

		int exitCode = application->Run();

		delete application;
		return exitCode;
	}
	catch (const EngineExcept::EngineException& e)
	{
		DEBUGLOG("An Engine Exception was caught in Entry Point.");
		MessageBox(nullptr, e.what(), e.GetExceptionType(), MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
		PostQuitMessage(-1);
	}
	catch (const std::exception& e)
	{
		DEBUGLOG("An Standard Libary Exception was caught in Entry Point.");
		MessageBox(nullptr, e.what(), "Standard Libary Exception", MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
		PostQuitMessage(-1);
	}
	catch (...)
	{
		DEBUGLOG("An Unknown Exception was caught in Entry Point.");
		MessageBox(nullptr, "Exception Type Unknown...", "Exception Type Unknown...", MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
		PostQuitMessage(-1);
	}

	return -1;
}