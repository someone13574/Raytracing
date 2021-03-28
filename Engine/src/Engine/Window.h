#pragma once

#include "Win.h"
#include "Macros.h"
#include "Input/Keyboard.h"
#include "Input/Mouse.h"

#include "Graphics/Graphics.h"

#include "BoundingVolumeHierarchy.h"
#include "MeshManager.h"

#include <optional>
#include <memory>

namespace Engine
{
	class ENGINE_DLL_DS Window
	{
	private:
		class WindowClass
		{
		public:
			static const char* GetName();
			static HINSTANCE GetInstance();
		private:
			WindowClass();
			~WindowClass();
			WindowClass(const WindowClass&) = delete;
			WindowClass& operator = (const WindowClass&) = delete;
			static constexpr const char* className = "Engine Window";
			static WindowClass windowClass;
			HINSTANCE instance;
		};
	public:
		Window(int width, int height, const char* name);
		~Window();
		Window(const Window&) = delete;
		Window& operator = (const Window&) = delete;
	public:
		int GetWidth() const;
		int GetHeight() const;
	public:
		void SetTitle(const char* title);
		static std::optional<int> ProcessMessages();
		Graphics::Graphics& GetGraphics();
	public:
		Input::Keyboard keyboard;
		Input::Mouse mouse;
		HWND window;

		MeshManagement::MeshManager meshManager;
		BoundingVolumeHierarchy boundingVolumeHierarchy;
	private:
		static LRESULT CALLBACK HandleMsgSetup(HWND window, UINT messgage, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK HandleMsgThunk(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK HandleMsg(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
	private:
		int width;
		int height;

#pragma warning(push)
#pragma warning(disable:4251)
		std::unique_ptr<Graphics::Graphics> pGraphics;
#pragma warning(pop)
	};
}