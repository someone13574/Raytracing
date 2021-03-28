#include "Application.h"

#include <chrono>
#include <sstream>
#include <math.h>
#include <iostream>

#include "EngineLogger.h"

namespace Engine
{
	Application::Application() : window(1920, 1080, "Engine Window")
	{

	}

	Application::~Application()
	{

	}

	void Application::Update()
	{

	}

	int Application::Run()
	{
		while (true)
		{
			if (const auto exitCode = Window::ProcessMessages())
			{
				return *exitCode;
			}

			Update();
		}
		
	}

	Graphics::Graphics* Application::GetGraphics()
	{
		return &window.GetGraphics();
	}

	Graphics::UIManager* Application::GetUIManager()
	{
		return &window.GetGraphics().uiManager;
	}

	Input::Keyboard* Application::GetKeyboard()
	{
		return &window.keyboard;
	}

	Input::Mouse* Application::GetMouse()
	{
		return &window.mouse;
	}

	void Application::SetTitle(const char* title)
	{
		window.SetTitle(title);
	}
}