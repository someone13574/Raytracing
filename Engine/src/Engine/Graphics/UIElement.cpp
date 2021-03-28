#include "UIElement.h"

#include <algorithm>
#include <math.h>
#include <time.h>

namespace Graphics
{
	UIElement::UIElement(unsigned int xPosition, unsigned int yPosition, unsigned int xScale, unsigned int yScale, float rChannel, float gChannel, float bChannel, float roundness)
	{
		currentState.xPosition = xPosition;
		currentState.yPosition = yPosition;
		currentState.xScale = xScale;
		currentState.yScale = yScale;
		currentState.r = rChannel;
		currentState.g = gChannel;
		currentState.b = bChannel;
		currentState.roundness = roundness;

		isAnimated = false;
		animationStartTime = 0;
		animationFunc = nullptr;

		UpdateState();
	}

	UIElement::UIElement(unsigned int xPosition, unsigned int yPosition, unsigned int xScale, unsigned int yScale, float rChannel, float gChannel, float bChannel, float roundness, GraphicsUIElement(*animationFunction)(AnimationEvent animationEvent, UIElement::GraphicsUIElement previousState, float elapsedSeconds), std::vector<float> animationLengths)
	{
		currentState.xPosition = xPosition;
		currentState.yPosition = yPosition;
		currentState.xScale = xScale;
		currentState.yScale = yScale;
		currentState.r = rChannel;
		currentState.g = gChannel;
		currentState.b = bChannel;
		currentState.roundness = roundness;

		isAnimated = true;
		animationStartTime = 0;
		this->animationLengths = animationLengths;
		animationFunc = animationFunction;

		UpdateState();

		PlayAnimation(AnimationEvent::Start);
	}

	float UIElement::IsTouching(int mouseXPosition, int mouseYPosition)
	{
		float dx = abs(((float)xPosition - (float)mouseXPosition)) - ((float)xScale - (float)roundness);
		float dy = abs(((float)yPosition - (float)mouseYPosition)) - ((float)yScale - (float)roundness);

		float cx = std::max(dx, 0.0f);
		float cy = std::max(dy, 0.0f);

		float outsideDistance = std::sqrt(cx * cx + cy * cy);
		float insideDistance = std::min(std::max(dx, dy), 0.0f);

		return outsideDistance + insideDistance - (float)roundness;
	}

	void UIElement::PlayAnimation(AnimationEvent animationEvent)
	{
		if (isAnimated)
		{
			while ((int)animationEvent >= animationLengths.size())
			{
				animationLengths.push_back(0);
			}

			animationStartTime = (float)clock() / CLOCKS_PER_SEC;
			isPlaying = true;
			lastEvent = animationEvent;
			currentState = animationFunc(animationEvent, currentState, 0);
			UpdateState();
		}
	}

	void UIElement::UpdateAnimation(float currentTime)
	{
		if (isAnimated && isPlaying)
		{
			float elapsedTime = currentTime - animationStartTime;

			if (elapsedTime > animationLengths.at(lastEvent))
			{
				isPlaying = false;
				return;
			}

			currentState = animationFunc(lastEvent, currentState, elapsedTime);
			UpdateState();
		}
	}

	bool UIElement::IsAnimated() const
	{
		return isAnimated;
	}

	bool UIElement::IsPlayingAnimation() const
	{
		return isPlaying;
	}

	UIElement::AnimationEvent UIElement::GetCurrentEvent() const
	{
		return lastEvent;
	}

	UIElement::GraphicsUIElement UIElement::GetElementData(bool *ioUpdateBuffer)
	{
		if (*ioUpdateBuffer == true && !bufferUpToDate)
		{
			*ioUpdateBuffer = false;
			bufferUpToDate = true;
		}
		return currentState;
	}

	void UIElement::UpdateState()
	{
		if (xPosition != currentState.xPosition || yPosition != currentState.yPosition || xScale != currentState.xScale || yScale != currentState.yScale || r != currentState.r || g != currentState.g || b != currentState.b || roundness != currentState.roundness)
		{
			bufferUpToDate = false;

			xPosition = currentState.xPosition;
			yPosition = currentState.yPosition;
			xScale = currentState.xScale;
			yScale = currentState.yScale;
			r = currentState.r;
			g = currentState.g;
			b = currentState.b;
			roundness = currentState.roundness;
		}
	}
}