#include "UIManager.h"

#include <algorithm>
#include <math.h>
#include <time.h>

namespace Graphics
{
	UIManager::UIManager(Input::Mouse* mouse) : pMouse(mouse)
	{

	}

	void UIManager::AddUIElement(UIElement element)
	{
		uiElements.push_back(element);
		upToDate = false;
	}

	void UIManager::SetUIElement(unsigned int index, UIElement element)
	{
		uiElements.at(index) = element;
		upToDate = false;
	}

	UIElement* UIManager::GetUIElement(unsigned int index)
	{
		return &uiElements.at(index);
	}

	std::vector<UIElement::GraphicsUIElement> UIManager::GetElementData()
	{
		std::vector<UIElement::GraphicsUIElement> outputData;

		float currentTime = (float)clock() / CLOCKS_PER_SEC;

		for (unsigned int i = 0; i < uiElements.size(); i++)
		{
			if (uiElements.at(i).IsAnimated())
			{
				bool isPlaying = uiElements.at(i).IsPlayingAnimation();
				UIElement::AnimationEvent currentEvent = uiElements.at(i).GetCurrentEvent();

				float dst = uiElements.at(i).IsTouching(pMouse->GetXPos(), pMouse->GetYPos());

				if (dst <= 0)
				{
					if (pMouse->isLeftDown())
					{
						uiElements.at(i).PlayAnimation(UIElement::AnimationEvent::Click);
					}
					else
					{
						if (!isPlaying || currentEvent == UIElement::AnimationEvent::Idle)
						{
							uiElements.at(i).PlayAnimation(UIElement::AnimationEvent::Hover);
						}
					}
				}
				else
				{
					if (!isPlaying || currentEvent == UIElement::AnimationEvent::Hover)
					{
						uiElements.at(i).PlayAnimation(UIElement::AnimationEvent::Idle);
					}
				}
				if (uiElements.at(i).IsPlayingAnimation())
				{
					uiElements.at(i).UpdateAnimation(currentTime);
				}
			}

			outputData.push_back(uiElements.at(i).GetElementData(&upToDate));
		}

		return outputData;
	}

	unsigned int UIManager::ElementCount() const
	{
		return static_cast<unsigned int>(uiElements.size());
	}

	void UIManager::MarkAsUpToDate()
	{
		numAllocatedElements = static_cast<unsigned int>(uiElements.size());
		upToDate = true;
	}

	bool UIManager::UpdateOnly()
	{
		return (uiElements.size() == numAllocatedElements);
	}

	bool UIManager::IsUpdateToDate() const
	{
		return upToDate;
	}
}