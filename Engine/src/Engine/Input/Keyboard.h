#pragma once

#include "../Macros.h"
#include "../../EngineStandard/Queue.h"

namespace Engine
{
	class Window;
}

namespace Input
{
	class ENGINE_DLL_DS Keyboard
	{
		friend class Engine::Window;
	public:
		class Event
		{
		public:
			enum class Type
			{
				Press,
				Release,
				Invalid
			};
		private:
			Type type;
			unsigned char code;
		public:
			Event() : type(Type::Invalid), code(0) {}
			Event(Type type, unsigned char code) : type(type), code(code) {}

			Type GetType() const
			{
				return type;
			}

			unsigned char GetCode() const
			{
				return code;
			}
		};
	public:
		Keyboard() = default;
		~Keyboard() = default;
		Keyboard(const Keyboard&) = default;
		Keyboard& operator=(const Keyboard&) = default;

		bool KeyDown(unsigned char keycode) const;

		Event GetEvent();
		bool EventQueueEmpty();
		void EmptyEventQueue();

		unsigned char GetChar();
		bool CharQueueEmpty();
		void EmptyCharQueue();

		bool autoRepeatEnabled = false;
	private:
		void SetKey(unsigned char keycode, bool pressed);
		void SetChar(char character);
		void ClearKeyStates();
	private:
		static constexpr unsigned int nKeys = 256;
		static constexpr unsigned int maxQueueLength = 16;
		bool states[256] = { false };
		ESL::Queue<Event> eventQueue;
		ESL::Queue<char> characterQueue;
	};
}