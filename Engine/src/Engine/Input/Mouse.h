#pragma once

#include "../Macros.h"
#include "../../EngineStandard/Queue.h"

#include <queue>

namespace Engine
{
	class Window;
}

namespace Input
{
	class ENGINE_DLL_DS Mouse
	{
		friend class Engine::Window;
	public:
		class Event
		{
		public:
			enum class Type
			{
				LPress,
				LRelease,
				RPress,
				RRelease,
				MPress,
				MRelease,
				WheelUp,
				WheelDown,
				Move,
				EnterWindow,
				ExitWindow,
				Invalid
			};
		public:
			Event() : type(Type::Invalid), leftDown(false), rightDown(false), middleDown(false), x(0), y(0)
			{

			}
			Event(Type type, const Mouse& mouseObj) : type(type), leftDown(mouseObj.leftDown), rightDown(mouseObj.rightDown), middleDown(mouseObj.middleDown), x(mouseObj.x), y(mouseObj.y)
			{

			}
		public:
			Type GetType() const
			{
				return type;
			}
			bool LeftButtonDown() const
			{
				return leftDown;
			}
			bool RightButtonDown() const
			{
				return rightDown;
			}
			bool MiddleButtonDown() const
			{
				return middleDown;
			}
			int GetXPos() const
			{
				return x;
			}
			int GetYPos() const
			{
				return y;
			}
		private:
			Type type;
			bool leftDown;
			bool rightDown;
			bool middleDown;
			int x;
			int y;
		};
	public:
		Mouse() = default;
		Mouse(const Mouse&) = delete;
		Mouse& operator = (const Mouse&) = delete;
	public:
		Mouse::Event ReadEvent();
		bool isQueueEmpty();
	public:
		int GetXPos() const;
		int GetYPos() const;
		bool isInWindow() const;
		bool isLeftDown() const;
		bool isRightDown() const;
		bool isMiddleDown() const;
	private:
		int x;
		int y;
		bool inWindow = false;
		bool leftDown = false;
		bool rightDown = false;
		bool middleDown = false;

		int wheelCarry = 0;
	private:
		static constexpr unsigned int maxQueueLength = 16;
		ESL::Queue<Mouse::Event> eventQueue;
	private:
		void OnMouseMove(int newx, int newy);
		void ExitWindow();
		void EnterWindow();
		void OnLeftAction(bool isDown);
		void OnRightAction(bool isDown);
		void OnMiddleAction(bool isDown);
		void OnWheelDelta(int delta);
	};
}