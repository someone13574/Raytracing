#include "Mouse.h"

#include "../Win.h"

namespace Input
{
	Mouse::Event Mouse::ReadEvent()
	{
		if (eventQueue.GetSize() > 0)
		{
			Mouse::Event e = eventQueue.Front();
			eventQueue.Dequeue();
			return e;
		}
		else
		{
			return Mouse::Event();
		}
	}

	bool Mouse::isQueueEmpty()
	{
		return eventQueue.isEmpty();
	}

	int Mouse::GetXPos() const
	{
		return x;
	}

	int Mouse::GetYPos() const
	{
		return y;
	}

	bool Mouse::isInWindow() const
	{
		return inWindow;
	}

	bool Mouse::isLeftDown() const
	{
		return leftDown;
	}

	bool Mouse::isRightDown() const
	{
		return rightDown;
	}

	bool Mouse::isMiddleDown() const
	{
		return middleDown;
	}

	void Mouse::OnMouseMove(int newx, int newy)
	{
		x = newx;
		y = newy;
		eventQueue.Enqueue(Mouse::Event(Mouse::Event::Type::Move, *this));
	}

	void Mouse::ExitWindow()
	{
		inWindow = false;

		eventQueue.Enqueue(Mouse::Event(Mouse::Event::Type::ExitWindow, *this));
	}

	void Mouse::EnterWindow()
	{
		inWindow = true;

		eventQueue.Enqueue(Mouse::Event(Mouse::Event::Type::EnterWindow, *this));
	}

	void Mouse::OnLeftAction(bool isDown)
	{
		leftDown = isDown;

		if (isDown)
		{
			eventQueue.Enqueue(Mouse::Event(Mouse::Event::Type::LPress, *this));
		}
		else
		{
			eventQueue.Enqueue(Mouse::Event(Mouse::Event::Type::LRelease, *this));
		}
	}

	void Mouse::OnRightAction(bool isDown)
	{
		rightDown = isDown;

		if (isDown)
		{
			eventQueue.Enqueue(Mouse::Event(Mouse::Event::Type::RPress, *this));
		}
		else
		{
			eventQueue.Enqueue(Mouse::Event(Mouse::Event::Type::RRelease, *this));
		}
	}

	void Mouse::OnMiddleAction(bool isDown)
	{
		middleDown = isDown;

		if (isDown)
		{
			eventQueue.Enqueue(Mouse::Event(Mouse::Event::Type::MPress, *this));
		}
		else
		{
			eventQueue.Enqueue(Mouse::Event(Mouse::Event::Type::MRelease, *this));
		}
	}

	void Mouse::OnWheelDelta(int delta)
	{
		wheelCarry += delta;
		while (wheelCarry >= WHEEL_DELTA)
		{
			wheelCarry -= WHEEL_DELTA;
			eventQueue.Enqueue(Mouse::Event(Mouse::Event::Type::WheelUp, *this));
		}
		while (wheelCarry <= -WHEEL_DELTA)
		{
			wheelCarry += WHEEL_DELTA;
			eventQueue.Enqueue(Mouse::Event(Mouse::Event::Type::WheelDown, *this));
		}
	}
}