#pragma once

#include "Macros.h"
#include "Window.h"

namespace Engine
{
	class ENGINE_DLL_DS Application
	{
	public:
		Application();
		virtual ~Application();
	public:
		int Run();
		virtual void Update();
	public:
		Graphics::Graphics* GetGraphics();
		Graphics::UIManager* GetUIManager();
		Input::Keyboard* GetKeyboard();
		Input::Mouse* GetMouse();
	public:
		void SetTitle(const char* title);\
	private:
		Engine::Window window;
	};

	Application* CreateApplication();
}

