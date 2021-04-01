#include "Window.h"
#include "Exceptions/EngineWndHresultException.h"
#include "EngineLogger.h"

#include <sstream>

using namespace std;

namespace Engine
{
	//Window Class
	Window::WindowClass Window::WindowClass::windowClass;

	Window::WindowClass::WindowClass() : instance(GetModuleHandle(nullptr))
	{
		WNDCLASSEX wndClass = { 0 };
		wndClass.cbSize = sizeof(wndClass);
		wndClass.style = CS_OWNDC;
		wndClass.lpfnWndProc = HandleMsgSetup;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = GetInstance();
		wndClass.hIcon = nullptr;
		wndClass.hCursor = nullptr;
		wndClass.hbrBackground = nullptr;
		wndClass.lpszMenuName = nullptr;
		wndClass.lpszClassName = GetName();
		wndClass.hIconSm = nullptr;

		RegisterClassEx(&wndClass);
	}

	Window::WindowClass::~WindowClass()
	{
		UnregisterClass(className, GetInstance());
	}

	const char* Window::WindowClass::GetName()
	{
		return className;
	}

	HINSTANCE Window::WindowClass::GetInstance()
	{
		return windowClass.instance;
	}

	//Window
	Window::Window(int width, int height, const char* name) : width(width), height(height)
	{
		meshManager.ReadMeshFile("C:/Users/Owen/Documents/C++/RaytracingEngine/Meshfiles/Dragon50k.stl");

		try
		{
			window = CreateWindow(WindowClass::GetName(), name, WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, WindowClass::GetInstance(), this);
			if (window == nullptr)
				WND_LAST_EXCEPT();
			ShowWindow(window, SW_SHOWDEFAULT);
			pGraphics = std::make_unique<Graphics::Graphics>(window, &mouse, width, height, &meshManager);
		}
		catch (const EngineExcept::EngineException& e)
		{
			ShowWindow(window, SW_HIDE);
			DEBUGLOG("An Engine Exception was caught in Window constructor.");
			MessageBox(nullptr, e.what(), e.GetExceptionType(), MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
			PostQuitMessage(-1);
			ExitProcess(-1);
		}
		catch (const std::exception& e)
		{
			ShowWindow(window, SW_HIDE);
			DEBUGLOG("An Standard Libary Exception was caught in Window constructor.");
			MessageBox(nullptr, e.what(), "Standard Libary Exception", MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
			PostQuitMessage(-1);
			ExitProcess(-1);
		}
		catch (...)
		{
			ShowWindow(window, SW_HIDE);
			DEBUGLOG("An Unknown Exception was caught in Window constructor.");
			MessageBox(nullptr, "Exception Type Unknown...", "Exception Type Unknown...", MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND);
			PostQuitMessage(-1);
			ExitProcess(-1);
		}
	}

	Window::~Window()
	{
		DestroyWindow(window);
	}

	int Window::GetWidth() const
	{
		return width;
	}

	int Window::GetHeight() const
	{
		return height;
	}

	void Window::SetTitle(const char* title)
	{
		if (SetWindowText(window, title) == 0)
			throw WND_LAST_EXCEPT();
	}

	std::optional<int> Window::ProcessMessages()
	{
		MSG message;
		while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
		{

			if (message.message == WM_QUIT)
				return (int)message.wParam;

			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		return {};
	}

	Graphics::Graphics& Window::GetGraphics()
	{
		if (!pGraphics)
			DEBUGERROR("No Graphics!");
		return *pGraphics;
	}

	//Message Handling
	LRESULT CALLBACK Window::HandleMsgSetup(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == WM_NCCREATE)
		{
			const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
			Window* const pWindow = static_cast<Window*>(pCreate->lpCreateParams);
			SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
			SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
			return pWindow->HandleMsg(window, message, wParam, lParam);
		}

		return DefWindowProc(window, message, wParam, lParam);
	}

	LRESULT CALLBACK Window::HandleMsgThunk(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
	{
		Window* const pWindow = reinterpret_cast<Window*>(GetWindowLongPtr(window, GWLP_USERDATA));
		return pWindow->HandleMsg(window, message, wParam, lParam);
	}

	LRESULT CALLBACK Window::HandleMsg(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_KILLFOCUS:
		{
			keyboard.ClearKeyStates();
			break;
		}
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			if (!(lParam & 0x40000000) || keyboard.autoRepeatEnabled)
				keyboard.SetKey(static_cast<unsigned char>(wParam), true);
			break;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			keyboard.SetKey(static_cast<unsigned char>(wParam), false);
			break;
		}
		case WM_CHAR:
		{
			keyboard.SetChar(static_cast<unsigned char>(wParam));
			break;
		}
		case WM_MOUSEMOVE:
		{
			const POINTS point = MAKEPOINTS(lParam);

			if (point.x >= 0 && point.x < width && point.y >= 0 && point.y < height)
			{
				mouse.OnMouseMove(point.x, point.y);
				if (!mouse.isInWindow())
				{
					SetCapture(window);
					mouse.EnterWindow();
				}
			}
			else
			{
				if (wParam & (MK_LBUTTON | MK_RBUTTON))
				{
					mouse.OnMouseMove(point.x, point.y);
				}
				else
				{
					ReleaseCapture();
					mouse.ExitWindow();
				}
			}
			break;
		}
		case WM_LBUTTONDOWN:
		{
			mouse.OnLeftAction(true);
			break;
		}
		case WM_LBUTTONUP:
		{
			mouse.OnLeftAction(false);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			mouse.OnRightAction(true);
			break;
		}
		case WM_RBUTTONUP:
		{
			mouse.OnRightAction(false);
			break;
		}
		case WM_MBUTTONDOWN:
		{
			mouse.OnMiddleAction(true);
			break;
		}
		case WM_MBUTTONUP:
		{
			mouse.OnMiddleAction(false);
			break;
		}
		case WM_MOUSEHWHEEL:
		{
			const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
			mouse.OnWheelDelta(delta);
			break;
		}
		}

		return DefWindowProc(window, message, wParam, lParam);
	}
}