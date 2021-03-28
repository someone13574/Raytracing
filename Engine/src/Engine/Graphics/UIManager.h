#pragma once

#include "UIElement.h"
#include "../Input/Mouse.h"
#include "../Macros.h"

#include <vector>

#pragma warning(push)
#pragma warning(disable:4251) //Disable DLL-Interface Warning

namespace Graphics
{
	class ENGINE_DLL_DS UIManager
	{
	public:
		UIManager(Input::Mouse* mouse);
		~UIManager() = default;
		UIManager(const UIManager&) = default;
		UIManager& operator = (const UIManager&) = default;
	public:
		void AddUIElement(UIElement element);
		void SetUIElement(unsigned int index, UIElement element);
		UIElement* GetUIElement(unsigned int index);
	public:
		std::vector<UIElement::GraphicsUIElement> GetElementData();
		unsigned int ElementCount() const;
		bool UpdateOnly();
		bool IsUpdateToDate() const;
		void MarkAsUpToDate();
	private:
		Input::Mouse* pMouse = nullptr;
	private:
		bool upToDate = false;
		unsigned int numAllocatedElements = 0; //Number of elements when buffer last updated
		std::vector<UIElement> uiElements;
	};
}

#pragma warning(pop)