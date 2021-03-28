#pragma once

#include "../Engine/Macros.h"

namespace ESL
{
	template <class T> class ENGINE_DLL_DS Queue
	{
	public:
		Queue(int size = 16)
		{
			arr = new T[size];
			capacity = size;
			frontIndex = 0;
			rearIndex = -1;
			count = 0;
		}
		~Queue()
		{
			delete[] arr;
		}

		void Dequeue()
		{
			if (isEmpty())
			{
				return;
			}

			frontIndex = (frontIndex + 1) % capacity;
			count--;
		}

		void Enqueue(T item)
		{
			if (isFull())
			{
				Dequeue();
			}
			rearIndex = (rearIndex + 1) % capacity;
			arr[rearIndex] = item;
			count++;
		}

		T Front()
		{
			return arr[frontIndex];
		}

		int GetSize()
		{
			return count;
		}

		bool isEmpty()
		{
			return (count == 0);
		}

		bool isFull()
		{
			return (count == capacity);
		}
	private:
		T *arr;
		int capacity;
		int frontIndex;
		int rearIndex;
		int count;
	};
}

