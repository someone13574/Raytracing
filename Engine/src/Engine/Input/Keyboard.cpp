#include "Keyboard.h"

namespace Input
{
	bool Keyboard::KeyDown(unsigned char keycode) const
	{
		return states[keycode];
	}

	Keyboard::Event Keyboard::GetEvent()
	{
		if (eventQueue.GetSize() > 0)
		{
			Keyboard::Event e = eventQueue.Front();
			eventQueue.Dequeue();
			return e;
		}
		else
		{
			return Keyboard::Event();
		}
	}

	bool Keyboard::EventQueueEmpty()
	{
		return eventQueue.isEmpty();
	}

	void Keyboard::EmptyEventQueue()
	{
		eventQueue = ESL::Queue<Keyboard::Event>(maxQueueLength);
	}

	unsigned char Keyboard::GetChar()
	{
		if (characterQueue.GetSize() > 0)
		{
			unsigned char character = characterQueue.Front();
			characterQueue.Dequeue();
			return character;
		}
		else
		{
			return 0;
		}
	}

	bool Keyboard::CharQueueEmpty()
	{
		return characterQueue.isEmpty();
	}

	void Keyboard::EmptyCharQueue()
	{
		characterQueue = ESL::Queue<char>(maxQueueLength);
	}

	void Keyboard::SetKey(unsigned char keycode, bool down)
	{
		states[keycode] = down;
		eventQueue.Enqueue(Keyboard::Event((down) ? Keyboard::Event::Type::Press : Keyboard::Event::Type::Release, keycode));
	}

	void Keyboard::SetChar(char character)
	{
		characterQueue.Enqueue(character);
	}

	void Keyboard::ClearKeyStates()
	{
		bool states[256] = { false };
	}
}